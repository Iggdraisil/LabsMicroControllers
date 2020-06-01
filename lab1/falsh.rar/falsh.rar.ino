int btn_pin = 49;

void setup() {
  // put your setup code here, to run once:
  DDRF = 0xFF;
  PORTF = 0;
  pinMode(btn_pin, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(btn_pin) == LOW) {
    PORTF = 0;
    byte start = 10000000;
    byte finish = 00000010;
    delay(1100);
    PORTF = finish;
    delay(1100);
    PORTF = start;
    while(finish < start) {
      delay(1100);
      finish = finish << 1;
      PORTF = finish;
      delay(1100);
      start = start >> 1;
      PORTF = start;
    }
    PORTF = 0;
  }
}
