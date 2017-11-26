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
String printer = "";
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
Adafruit_GPS GPS(&Serial1);
void setup() {
  pinMode(13, OUTPUT);
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  
  rtc.begin();
  Serial.begin(9600);
  pinMode(53, OUTPUT);
  while(!SD.begin(4)){       
    Serial.println("no SD card biotch!");
    delay(300);
    digitalWrite(13, HIGH);
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
  digitalWrite(13, HIGH);
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
  logFile.println("sep=,");    //for some reason this needs to be added?
  logFile.println("sensor, pin, pulses, LPO, avg pulse length, minduration, maxduration, glighttime, altitude,gpstime ");
  Serial.println("file header added");
  GPSlog.println("sep=,");
  GPSlog.println("Flight Time, Lat, Long, Altitude (ft), Date, Hour:Min:Sec");
  logFile.close();
  GPSlog.close();
  Serial.println("GPS header added");                     
  for(int i = 0; i<2;i++){
    pinMode(Sensors[i].getLow(), INPUT);
    pinMode(Sensors[i].getHigh(), INPUT);
    attachInterrupt(digitalPinToInterrupt(Sensors[i].getLow()), readSen, CHANGE);
    attachInterrupt(digitalPinToInterrupt(Sensors[i].getHigh()), readSen, CHANGE);
  }
digitalWrite(13, LOW);

  
  
}

void loop() {
  while(Serial1.available()>0){
    GPS.read();
  }
  if(GPS.newNMEAreceived()){
    GPS.parse(GPS.lastNMEA());
  }
  if(millis()-GPStimer>GPS_SAMPLE){
    GPSlog = SD.open(GPSfilename, FILE_WRITE);
    String data = "";
    data += (flightTimeStr() + "," + String(GPS.latitudeDegrees, 6) + "," + String(GPS.longitudeDegrees, 6) + ",");
      data += (String(GPS.altitude * 3.28048) + ",");    //convert meters to feet for datalogging
      data += (String(GPS.month) + "/" + String(GPS.day) + "/" + String(GPS.year) + ",");
      data += (String(GPS.hour) + ":" + String(GPS.minute) + ":" + String(GPS.seconds));
    //gps data will go here
    //wee!
    Serial.println(data);
    GPSlog.println(data);
    GPSlog.close();
    GPStimer = millis();
  }
  if(change){
    for(int i =0;i<2;i++){
      Sensors[i].update();
    }
    change = false;
  }
  if(millis()-sampleTimer >SAMPLE){
    for(uint8_t i =0; i<2; i++){
      printer =(Sensors[i].reset(i));
      printer += ",";
      printer += String(GPS.altitude * 3.28048);
      printer += ",";
      printer += flightTimeStr();
      printer += ",";
      printer += (String(GPS.hour) + ":" + String(GPS.minute) + ":" + String(GPS.seconds));
      Serial.println(printer);
      logFile = SD.open(logfilename, FILE_WRITE);
      logFile.println(printer);
      logFile.close();
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
String flightTimeStr() {
  unsigned long t = millis() / 1000;
  String fTime = "";
  fTime += (String(t / 3600) + ":");
  t %= 3600;
  fTime += String(t / 600);
  t %= 600;
  fTime += (String(t / 60) + ":");
  t %= 60;
  fTime += (String(t / 10) + String(t % 10));
  return fTime;
}

