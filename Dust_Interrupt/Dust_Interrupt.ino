#include <Adafruit_GPS.h>
#include <RTClib.h>   
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
#include <SD.h>    // a modified library can be found at http://adafru.it/aP6 for the SD card for the mega
//declare {low,high} pins for the dustsensors. 3 is the max for interrupts.
dSen Sensors[2] = {{20,21},{9,10}};
RTC_DS1307 rtc;
//the Low and High states of the dust sensor
//pulse occupancy variables
bool set = false;
volatile bool change = false;
#define SAMPLE 30000
#define GPS_SAMPLE 1000
unsigned long GPStimer = 0;
unsigned long sampleTimer = 0;
char GPSfilename[] = "GPSlog00.csv";
char logfilename[] = "dLog00.csv";
File logFile;
File GPSlog;
SoftwareSerial GPSserial(18,19);  //are these the right pins??? maybe??
Adafruit_GPS GPS(&GPSserial);
void setup() {
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  
  rtc.begin();
  Serial.begin(9600);
  while(!SD.begin(10,11,12,13)){       
    Serial.println("no SD card biotch!");
  }
  Serial.println("SD card initialized");
  for(uint8_t i = 0; i<100; i++){
    GPSfilename[6] = i/10 + '0';
    GPSfilename[7] = i%10 + '0';
    if(!SD.exists(GPSfilename)){
      GPSlog = SD.open(GPSfilename, FILE_WRITE);
      break;
    }
  }
  Serial.println("GPS log created: " + String(GPSfilename));
  for(uint8_t i = 0; i<100; i++){
    logfilename[4] = i/10 + '0';
    logfilename[5] = i%10 + '0';
    if(!SD.exists(logfilename)){
      logFile = SD.open(logfilename, FILE_WRITE);
      break;
    }
  }
  Serial.println("Log file initialized: " + String(logfilename));
  logFile.println("sensor, pin, Date, Time, # of pulses, pulse occupancy, average duration, min duration");
  Serial.println("file header added");
  GPSlog.println("Date, Time, altitude (feet), latitude, longitude");
  logFile.close();
  GPSlog.close();
  Serial.println("GPS header added");                     
  for(int i = 0; i<2;i++){
    pinMode(Sensors[i].getLow(), INPUT);
    pinMode(Sensors[i].getHigh(), INPUT);
    attachInterrupt(digitalPinToInterrupt(Sensors[i].getLow()), readSen, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Sensors[i].getHigh()), readSen, CHANGE);
  }



  
  
}

void loop() {
  if(millis()-GPStimer>GPS_SAMPLE){
    GPSlog = SD.open(GPSfilename);
    //gps data will go here
    //wee!
    GPSlog.close();
  }
  if(change){
    for(int i =0;i<2;i++){
      Sensors[i].update();
    }
    change = false;
  }
  if(millis()-sampleTimer >SAMPLE){
    for(uint8_t i =0; i<2; i++){
      Serial.println(Sensors[i].reset(i));
    }
    sampleTimer = millis();
  }
}
void readSen(){
  for(int i = 0; i<2; i++){
    Sensors[i].checkStatus();
    change = true;
  }
}

