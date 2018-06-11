int setCoil = 2;
int resetCoil = 3;
void setup() {
  pinMode(setCoil, OUTPUT);
  pinMode(resetCoil,  OUTPUT);

}

void loop() {
  delay(2500);
  pulse(setCoil);
  delay(5000);
  pulse(resetCoil);
  delay(5000);

}

void pulse(int coilPin){
  digitalWrite(coilPin, HIGH);
  delay(20);
  digitalWrite(coilPin, LOW);
}

