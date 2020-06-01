const char symbols[] = {
  '1', '2', '3', 'A',
  '4', '5', '6', 'B',
  '7', '8', '9', 'C',
  '#', '0', '*', 'D'
};
static const uint8_t analog_pins[] = {A0, A1, A2, A3};
int start_time;
const int BUZZER_PIN = 7;

#include <LiquidCrystal.h>;
LiquidCrystal lcd(13, 12, 8, 9, 10, 11);

struct Time
{
  unsigned char second, minute, hour;
};

Time times[60];
int times_stored = 0;
bool pause = false;
unsigned long pause_time;

Time current_time = {0, 0, 0};

void displayTime(Time time1) {
  lcd.home();
  if (time1.hour < 10) {
    lcd.print(0);
  }
  lcd.print(time1.hour);
  lcd.print(':');
  if (time1.minute < 10) {
    lcd.print(0);
  }
  lcd.print(time1.minute);
  lcd.print(':');
  if (time1.second < 10) {
    lcd.print(0);
  }
  lcd.print(time1.second);
}

void setup() {
  // put your setup code here, to run once:
  for (int i = 2; i < 6; i++) {
    pinMode(analog_pins[i - 2], INPUT_PULLUP);
    pinMode(i, OUTPUT);
  }
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(9600);
  lcd.begin(16, 2);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);
  start_time = millis();
  displayTime(current_time);
}
char listenToKeyboard() {
  for (int i = 2; i < 6; i++) {
    digitalWrite(i, LOW);
  }
  for (int k = 0; k < 4; k++) {
    if (digitalRead(analog_pins[k]) == LOW) {
      for (int p = 2; p < 6; p++) {
        digitalWrite(p, HIGH);
      }
      for (int j = 2; j < 6; j++) {
        digitalWrite(j, LOW);
        if (digitalRead(analog_pins[k]) == LOW) {
          return symbols[k * 4 + j - 2];
        }
        digitalWrite(j, HIGH);
      }
    }
  }
  return 'n';
}

Time convertToTime(unsigned long milliseconds) {
  unsigned long currentTime = milliseconds - start_time;
  int hours = currentTime / 3600000;
  int minutes = (currentTime % 3600000) / 60000;
  int seconds = (currentTime % 60000) / 1000;
  Time now = {seconds, minutes, hours};
  return now;
}

void beep() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);
}

void long_beep() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);
}
void outputMemory(int i = 0) {
  if (times_stored) {
    displayTime(times[i]);
    lcd.setCursor(0, 1);
    lcd.print(sizeof("Memory"));
    lcd.print(i + 1);
    while (true) {
      char out = listenToKeyboard();
      if (out != 'n') {
        if (out == 'B') {
          if (++i < times_stored) {
            lcd.home();
            displayTime(times[i]);
            lcd.setCursor(0, 1);
            lcd.print(i + 1);
          }
          delay(300);
        }
        if (out == 'A') {
          delay(300);
          return;
        }
        if ((out - '0') < 10) {
          Serial.print(out - '0');
          delay(300);
          while (true) {
            char second = listenToKeyboard();
            if (second = 'n') {
                continue;
            }
            if (second = '#') {
                beep();
                delay(300);
                outputMemory(out - '0');
              }
            if ((second - '0') < 10) {
              int number = (out - '0') * 10 + (second - '0');
              while (true) {
                int next_one = listenToKeyboard();
                if (next_one = '#' && (number <= times_stored)) {
                  Serial.print(number);
                  delay(300);
                  outputMemory(number);
                } else {
                  long_beep();
                  delay(300);
                  outputMemory();
                }
              }
            } else {
                delay(300);
                outputMemory();
            }
          }
        }
      }
    }
  }
}

void loop() {
  //   put your main code here, to run repeatedly
  if (convertToTime(millis()).second != current_time.second && !pause) {
    if (convertToTime(millis()).minute != current_time.minute)
      long_beep();
    current_time = convertToTime(millis());
    displayTime(current_time);
  }
  char out = listenToKeyboard();
  if (out != 'n') {
    if (out == 'A') {
      beep();
      delay(1000);
      outputMemory();
    } else if (out == '#' && !pause) {
      Time now = convertToTime(millis());
      times[times_stored] = now;
      times_stored++;
      lcd.setCursor(13, 1);
      lcd.print('M');
      lcd.print(times_stored);
      beep();
      delay(200);
    } else if (out == '*') {
      times_stored = 0;
      beep();
      lcd.clear();
      delay(200);
    } else if (out == 'D') {
      if (pause) {
        start_time += (millis() - pause_time);
        pause = false;
      } else {
        pause = true;
        pause_time = millis();
      }
    } else if (out == 'C') {
      if (!pause) {
        pause = true;
        start_time = millis();
        pause_time = millis();
        displayTime({0, 0, 0});
      }
    }
  }
}
