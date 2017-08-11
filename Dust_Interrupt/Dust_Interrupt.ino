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
dSen Sensors[2] = {{7,8},{9,10}};


void setup() {
  // put your setup code here, to run once:
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
