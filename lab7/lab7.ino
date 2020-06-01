int start_time;
const int BUZZER_PIN = 29;
const byte numbers[10] = {
  0b00111111,
  0b00000110,
  0b01011011,
  0b01001111,
  0b01100110,
  0b01101101,
  0b01111101,
  0b00000111,
  0b01111111,
  0b01101111
};

struct Time
{
  int second, minute, hour;
};

Time times[10];
int times_stored = 0;
bool pause = false;
unsigned long pause_time;
bool counting = true;

Time current_time = {0, 0, 0};
  int prev = 0;
  int prev2 = 1;

void displayTime(Time time1 = current_time) {
  int h1 = time1.hour / 10;
  int m1 = time1.minute / 10;
  int s1 = time1.second / 10;
  
  PORTD = 0b11111110;
  PORTB = numbers[h1];
  delay(2);
  PORTD = 0b11111101;
  PORTB = numbers[time1.hour % 10];

  delay(2);
  PORTD = 0b11111011;
  PORTB = numbers[m1];

  delay(2);
  PORTD = 0b11110111;
  PORTB = numbers[time1.minute % 10];

//  delay(2);
//  PORTD = 0b11101111;
//  PORTB = numbers[s1];
//
//  delay(2);
//  PORTD = 0b11011111;
//  PORTB = numbers[time1.second % 10];
  delay(2);

}

int listenToKeyboard() {
  if (digitalRead(22) == LOW)
    return 1;
  else if (digitalRead(23) == LOW)
    return 2;
  else if (digitalRead(24) == LOW)
    return 3;
  else if (digitalRead(25) == LOW)
    return 4;
  return 0;
}

void beep(Time this_time = current_time) {
  digitalWrite(BUZZER_PIN, HIGH);
  for (int j = 0; j<16; j++) {
    displayTime(this_time);
  }
  digitalWrite(BUZZER_PIN, LOW);
}

void long_beep(Time this_time = current_time) {
  digitalWrite(BUZZER_PIN, HIGH);
  for (int j = 0; j<80; j++) {
    displayTime(this_time);
  }
  digitalWrite(BUZZER_PIN, LOW);
}

void outputMemory(int i = 0) {
  for (;;) {
    displayTime(times[i]);
    if (digitalRead(25) == LOW) {
      if (++i < times_stored) {
        beep(times[i]);
        displayTime(times[i]);
      } else {
        long_beep();
        counting = true;
        break;
      }
    } else {
      if (digitalRead(22) == LOW) {
        current_time = {0, 0, 0};
        counting = true;
        beep();
        return;
      }
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  DDRA = 0b10000000;
  PORTA = 0b00001111;
  DDRB = 0b11111111;
  PORTB = 0b00000000;
  DDRD = 0b00111111;
  PORTD = 0b11111111;
  long_beep();
  noInterrupts();
  TIMSK2 = 0;
  ASSR |= (1 << AS2);
  TCNT2 = 0; 
  TCCR2B = 0x00; 
  TCCR2B |= (1 << CS22) /*| (1 << CS20)*/; // Prescale 128 
  while (ASSR & ((1 << TCN2UB) | (1 << TCR2BUB)));
  TIFR2  = (1 << TOV2);
  TIMSK2  = (1 << TOIE2);
  interrupts();
}

void loop() {
  displayTime();
//  int out = listenToKeyboard();
//  if (out != 0) {
//    if (out == 2) {
//      times[times_stored] = current_time;
//      times_stored++;
//      beep();
//    } else if (out == 3) {
//      times_stored = 0;
//      beep();
//      delay(200);
//      return;
//    } else if (out == 1) {
//      counting = false;
//      beep();
//      while (1) {
//        displayTime(current_time);
//        if (digitalRead(22) == LOW) {
//          current_time = {0, 0, 0};
//          counting = true;
//          beep();
//          break;
//        } else {
//          if (digitalRead(25) == LOW) {
//            beep();
//            if (times_stored > 0) {
//              outputMemory();
//            }
//            break;
//          }
//        }
//      }
//    } else if (out == 4) {
//    long_beep();
//    }
//  }
}

ISR(TIMER2_OVF_vect) {
    int temp = prev2;
    prev2 +=prev;
    prev = temp; 
    current_time = {0, temp%100, temp/100};
  /*if (counting) {
    current_time.second++;
    if (current_time.second == 60) {
      current_time.second = 0;
      current_time.minute++;
      beep();
      if (current_time.minute == 60) {
        current_time.minute = 0;
        current_time.hour++;
        if (current_time.hour == 24) {
          current_time.hour = 0;
        }
      }
    }
  }*/
}
