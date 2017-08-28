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
//pulse occupancy variables
bool set = false;
volatile bool change = false;


void setup() {
  Serial.begin(9600);
  
  for(int i = 0; i<2;i++){
    pinMode(Sensors[i].getLow(), INPUT);
    pinMode(Sensors[i].getHigh(), INPUT);
    attachInterrupt(digitalPinToInterrupt(Sensors[i].getLow()), readSen, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Sensors[i].getHigh()), readSen, CHANGE);
  }



  
  
}

void loop() {
  if(change){
    for(int i =0;i<2;i++){
      Sensors[i].update();
    }
    change = false;
  }
}
void readSen(){
  for(int i = 0; i<2; i++){
    Sensors[i].checkStatus();
    change = true;
  }
}

