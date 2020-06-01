const int btn_pin1 = 12;
const int btn_pin2 = 13;
const int delay_time = 1100;

void execute_algorithm_2() {
    PORTD = 0;
    byte start = B10000000;
    byte finish = B00000100;
    PORTD = finish;
    delay(delay_time);
    PORTD = start;
    while(finish < start) {
      delay(delay_time);
      finish = finish << 1;
      PORTD = finish;
      delay(delay_time);
      start = start >> 1;
      PORTD = start;
    }
    PORTD = 0;
}

void execute_algorithm_1() {
    PORTD = 0;
    byte start = B00000100;
    int counter = 6;
    for (int counter = 6; counter >0; counter --) {
      PORTD = start;
      delay(delay_time);
      start = start <<1;
    }
    PORTD = 0;
}

void setup() {
  // put your setup code here, to run once:
  DDRD = 0xFF;
  PORTD = 0;
  pinMode(btn_pin1, INPUT_PULLUP);
  pinMode(btn_pin2, INPUT_PULLUP);
  Serial.begin(19200);
}

void loop() {
  // put your main code here, to run repeatedly:
  byte first = 0xA1;
  byte second =0xB1;
  if (digitalRead(btn_pin1) == LOW) {
    Serial.write(first);
    delay(200);
  }
  if (digitalRead(btn_pin2) == LOW) {
    Serial.write(second);
    delay(200);
  }
  if (Serial.available() > 0) {
    byte command = Serial.read();
    if (command == 0xA1) {
      execute_algorithm_1();
    } else if (command == 0xB1) {
      execute_algorithm_2();
    }
  }
 
}
