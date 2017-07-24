/*
 D. Brooks, December 2015
 Interface to Telaire SMART Dust Sensor SM-PWM-01A (see Mouser Electronics 527-SM-PWM-01A)
 See http://www.amphenol-sensors.com/en/products/co2/co2-modules/3222-telaire-smart-dust-sensor#download
   for data sheet and application notes.
 See https://www.arduino.cc/en/Reference/PulseIn for pulseIn() function.
 Pins labeled 1-5, right to left looking down on top of device, black housing at right
   (rightmost pin is GND).
 pin 1 = GND
 pin 2 = P2 signal out (large particles, 3~10 um)
 pin 3 = +5V
 pin 4 = P1 signal out (small particles, 1~2 um)
 pin 5 no connection
 Run the fan from the +3.3V pin and GND on the Arduino.
 Connect the DHT22 sensor red/black wires to +5V/GND and the yellow wire to D8.
*/
#define ECHO_TO_FILE 1
#define ECHO_TO_SERIAL 1
#define SDError 3
#define FileError 4
#include <Wire.h>
#include "RTClib.h"
#include <SoftwareSerial.h>
 
#include <Adafruit_GPS.h>
Adafruit_GPS GPS(&Serial1);
//uint32_t timer = millis();                  //GPS timer
 
RTC_DS1307 rtc;
#include <SPI.h>
#include <SD.h>
File logfile;
char filename[] = "LOGBLK00.CSV";
const int  chipSelect = 53, lineFeed = 0;
const int num_detectors = 4; //specifiy number of particle detectors being used
const int D1[2] = {7, 6}; //specify input pins for detector 1 (P1, P2)
const int D2[2] = {8, 9}; //specify input pins for detector 2 (P1, P2)
const int D3[2] = {12, 13}; //specify input pins for detector 3 (P1, P2)
const int D4[2] = {12, 13}; //specify input pins for detector 4 (P1, P2)
//const int Di[2] = [P1, P2] gives template for an i'th detector using pins P1 and P2
int pin, n_pulses = 0;
const int readLED = 3;
const int logLED = 4;
unsigned long duration, min_duration = 1000000, max_duration = 0;
unsigned long starttime;
// unsigned long waittime=500000; // microseconds to wait for pulse to complete before return from pulseIn()
unsigned long sampletime_ms = 5000; //sample 30 s
unsigned long lowpulseoccupancy = 0;
float ratio, concentration;
int SDStatus = 0;
 
int increment;
void setup() {
  Serial.begin(9600);
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  delay(1000);
  Serial1.println(PMTK_Q_RELEASE);
  Wire.begin(); rtc.begin();
  
  pinMode(D1[0], INPUT); pinMode(D1[1], INPUT);
  pinMode(D2[0], INPUT); pinMode(D2[1], INPUT);
  pinMode(D3[0], INPUT); pinMode(D3[1], INPUT);
  pinMode(D4[0], INPUT); pinMode(D4[1], INPUT);
  pinMode(readLED, OUTPUT);
  pinMode(logLED, OUTPUT);
  starttime = millis();//get the current time;
#if ECHO_TO_FILE
 SDsetup();
    // Long blink the LED to inidicate successful SD initialization
    for(int i=0; i < 4; i++)
    {
      digitalWrite(logLED, HIGH);
      delay(1000);
      digitalWrite(logLED, LOW);
      delay(1000);
    }
  
  logfile.println("ch,Date,Time,pulse_occupancy,avg_duration,min_duration,max_duration, GPS_altitude, GPS_time");
  logfile.flush();
#endif
#if ECHO_TO_SERIAL
  Serial.println("ch,Date,Time,#_pulses,pulse_occupancy,avg_duration,min_duration,max_duration, GPS_altitude, GPS_time");
#endif
}
void loop() {
  increment = 0;
  starttime = millis();
  readPulses(D1[0]);
  increment ++;
  readPulses(D2[0]);
  increment ++;
  readPulses(D3[0]);
  increment ++;
  readPulses(D4[0]);
  increment ++;
  //insert the following code for i'th detector here:
  //readPulses(Di[0];
  //increment ++;
  starttime = millis();
  readPulses(D1[1]);
  increment ++;
  readPulses(D2[1]);
  increment ++;
  readPulses(D3[1]);
  increment ++;
  readPulses(D4[1]);
  increment ++;
  //insert the following code for i'th detector here:
  //readPulses(Di[1];
  //increment ++;
}

void readPulses(int P) {
 
  pin = P;
  do {
    duration = pulseIn(pin, LOW); // pulseIn() returns pulse length, microseconds
    if (duration > 0) {
      n_pulses++;
      if (duration < min_duration) min_duration = duration;
      if (duration > max_duration) max_duration = duration;
      lowpulseoccupancy += duration;
    }
  } while ( (millis() - starttime) < sampletime_ms) ;
  // Blink the read LED to indicate a pulse read
  LEDBlink(readLED);
 
#if ECHO_TO_SERIAL
  Serial.print("D" + (String)(increment % num_detectors + 1) + "[" + (String)(increment / num_detectors + 1) + "]:, ");
#endif
#if ECHO_TO_FILE
  logfile.print("D" + (String)(increment % num_detectors + 1) + "[" + (String)(increment / num_detectors + 1) + "]:, ");
#endif
 
  if (n_pulses > 0) {
    ratio = 0.1 * lowpulseoccupancy / sampletime_ms; // pulse occupancy percentage 0<=100
 
#if ECHO_TO_SERIAL
    DisplayTime(lineFeed);
    Serial.print(n_pulses); Serial.print(", "); Serial.print(ratio, 2); Serial.print(", ");
    Serial.print((float)lowpulseoccupancy / n_pulses, 1); Serial.print(", ");
    Serial.print(min_duration); Serial.print(", ");  Serial.print(max_duration); Serial.print(", ");
 
 
#endif
#if ECHO_TO_FILE
    WriteTime(lineFeed);
    logfile.print(n_pulses); logfile.print(", "); logfile.print(ratio, 2); logfile.print(", ");
    logfile.print((float)lowpulseoccupancy / n_pulses, 1); logfile.print(", ");
    logfile.print(min_duration); logfile.print(", "); logfile.print(max_duration); logfile.print(", ");
    // Indicate writing to the file by flashing log LED if correct initialization occured
    if(SDStatus)
    {
      LEDBlink(logLED);
    }

#endif
    get_gps();
#if ECHO_TO_SERIAL
    Serial.println();
#endif
#if ECHO_TO_FILE
    logfile.println();
    logfile.flush();
#endif
    n_pulses = 0;
  }
  else {
#if ECHO_TO_SERIAL
    DisplayTime(lineFeed);
    Serial.print("0, 0, 0, 0, 0, ");
 
#endif
#if ECHO_TO_FILE
    WriteTime(lineFeed);
    logfile.print("0, 0, 0, 0, 0, ");
    if(SDStatus)
    {
      LEDBlink(logLED);
    }
 
#endif
    get_gps();
#if ECHO_TO_SERIAL
    Serial.println();
#endif
#if ECHO_TO_FILE
    logfile.println();
    logfile.flush();
#endif
  }
  min_duration = 1000000; max_duration = 0; lowpulseoccupancy = 0;
}
void WriteTime(boolean lineFeed) { // to SD card file buffer
  DateTime now = rtc.now();
  logfile.print(now.year());   logfile.print('/');
  logfile.print(now.month());  logfile.print('/');
  logfile.print(now.day());    logfile.print(',');
  logfile.print(now.hour());   logfile.print(':');
  logfile.print(now.minute()); logfile.print(':');
  logfile.print(now.second()); logfile.print(", ");
  if (lineFeed) logfile.println();
  else logfile.print(", ");
}
void DisplayTime(boolean lineFeed) {
  DateTime now = rtc.now();
  Serial.print(now.year());   Serial.print('/');
  Serial.print(now.month());  Serial.print('/');
  Serial.print(now.day());    Serial.print(',');
  Serial.print(now.hour());   Serial.print(':');
  Serial.print(now.minute()); Serial.print(':');
  Serial.print(now.second()); Serial.print(", ");
  if (lineFeed) Serial.println();
  else Serial.print(", ");
}

// Upon SD initialization failure this function enters into a loop until the SD initialization failure is corrected
// The LEDs will continuously blink until corrected
void SDsetup() {
  bool SDCheck = 0;
  // initialize the SD card
  Serial.println("Initializing SD card:");
  while(!SDCheck)
  {
    // make sure that the default chip select pin is set to
    // output, even if you don't use it:
    pinMode(10, OUTPUT);
    // see if the card is present and can be initialized:
    SDCheck = 1;
    if (!SD.begin(chipSelect)) {
      Serial.println("SD Card failed");
      // blink SD LED to indicate the SD init failed 
      ErrorBlink(SDError);
      SDCheck = 0;
    }
  }
  Serial.println("SD Card Initialized");
  // create a new file
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
  }
  if (! logfile) {
    Serial.println("couldn't create file");
    // Blink the log LED to indicate file creation fail
    while(1){
      // Blink the LED to indicate the file could not be opned and action should be taken to fix before launch
      ErrorBlink(FileError);
    }
  }
  Serial.print("Logging to: "); Serial.println(filename);
  return 1;
}
 
void get_gps() {
  char c = GPS.read();
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }
  //if (millis() - timer > 10000) { //print the altitude every 10 secconds
  //timer = millis(); //reset the timer
#if ECHO_TO_SERIAL
  Serial.print(GPS.altitude, 3);
  Serial.print(",");
  Serial.print(GPS.hour);
  Serial.print(":");
  Serial.print(GPS.minute);
  Serial.print(":");
  Serial.print(GPS.seconds);
  Serial.print(" Fix:");
  Serial.print(GPS.fix);
  Serial.print(" Satellites:");
  Serial.print(GPS.satellites);
#endif
#if ECHO_TO_FILE
  logfile.print(GPS.altitude, 3);
  logfile.print(",");
  logfile.print(GPS.hour);
  logfile.print(":");
  logfile.print(GPS.minute);
  logfile.print(":");
  logfile.print(GPS.seconds);
#endif
  //}
}

// Function to blink LEDs in an error pattern
// Alternate the blinking of the log and read LEDs 
void ErrorBlink(int Error)
{
  // Blink both LEDs to signal error
  for(int i=0; i < 5; i++)
  {
    digitalWrite(logLED, HIGH);
    digitalWrite(readLED, LOW);
    delay(200);
    digitalWrite(logLED, LOW);
    digitalWrite(readLED, HIGH);
    delay(200);
  }
  digitalWrite(readLED, LOW);
  delay(1000);
  // Blink the log LED a set number of times (Error) to indicate the error
  for(int i=0; i < Error; i++)
  {
    digitalWrite(logLED, HIGH);
    delay(500);
    digitalWrite(logLED, LOW);
    delay(500);
  }
}

void LEDBlink(int pin)
{
  digitalWrite(pin, HIGH);
  delay(100);
  digitalWrite(pin, LOW);
}


