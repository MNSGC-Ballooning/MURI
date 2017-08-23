#include <BlackDust.h>
/*
 Simon Peterson, August 2017
 Writing code to use the Telaire Smart Dust Sensor using interrupts to allow for multiple
 sensors to be used.
 pin 1 = GND
 pin 2 = P2 signal out (large particles, 3~10 um)
 pin 3 = +5V
 pin 4 = P1 signal out (small particles, 1~2 um)
 pin 5 no connection
 */
#include <SPI.h>
#include <SD.h>
//declare {low,high} pins for the dustsensors
dSen Sensors[2] = {{20,21},{9,10}};
//the Low and High states of the dust sensor
volatile byte state1;
volatile byte state2;
//pulse occupancy variables
bool set = false;




void setup() {
  pinMode(Sensors[0].getLow(), INPUT);
  pinMode(Sensors[1].getLow(), INPUT);
  for(int i = 0; i<2;i++){
    attachInterrupt(digitalPinToInterrupt(Sensors[i].getLow()), readSen, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Sensors[i].getHigh()), readSen, CHANGE);
  }



  Serial.begin(9600);
  
}

void loop() {
for(int i = 0; i<2; i++){
  if(Sensors[i].changed){
    Sensors[i].changed = false;
    Serial.println("we are changing!");
  }
}
}
void readSen(){
for(int i = 0; i < 2; i++){
  Sensors[i].setLowState(digitalRead(Sensors[i].getLow()));
  Sensors[i].setHighState(digitalRead(Sensors[i].getHigh()));
  if(Sensors[i].getLowState()!=Sensors[i].getPrevLow())
  {
    Sensors[i].changed = true;
    break;
  }
  if(Sensors[i].getHighState()!=Sensors[i].getPrevHigh()){
    Sensors[i].changed = true;
    break;
  }
}
  
}


