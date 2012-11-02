// include the library code:
#include <SerialLCD.h>
#include <SoftwareSerial.h> //this is a must
#include <Wire.h>
#include <barometer085.h>
#include <dht11.h>
#include <tp401a.h>

#define SOUND_PORT A3
#define DHT11_PORT 4
#define BAROM_ID 119
#define GAS_PORT A2

// initialize the library
SerialLCD slcd(2,3); //this is a must, assign soft serial pins
Barometer barom(BAROM_ID);
dht11 dht11Sensor;
tp401a tp401aSensor(GAS_PORT);

unsigned long savedTemp = 0;
float savedBaromTemp = 0;
float savedBaromHumid = 0;
long savedPress = 0;
int savedHumid;
double savedDewPoint = 0;
float lastPressureValue = 0;

unsigned long lastBaromCheck = 0;
unsigned long lastHumidCheck = 0;
unsigned long airAvg = 0;

unsigned long lastSoundMessage = 0;
unsigned long lastAirMessage = 0;

unsigned long lastScroll = 0;
unsigned long milliCounter = 0;
unsigned long soundCounter = 0;



unsigned long avgSound;
unsigned long totalSound;
unsigned long maxSound;

unsigned long lastAirRead = 0;

//////

void setup() {
 
  Serial.begin(9600);
  // I2c setup
  Wire.begin();

  // Map active Wire object to barometer.
  barom.begin(Wire);

  // set up LCD
  slcd.begin();

  // Turn on screen light
//  slcd.setCursor(0, 0);
//  slcd.backlight();
//  slcd.setCursor(0, 0);
}

void loop() {

  float tempBaromTemp = 0;
  long tempPress = 0;
  
  milliCounter = millis();
  
  unsigned long soundValue = analogRead(SOUND_PORT);
  if(soundValue > maxSound) {
    maxSound = soundValue;   
  }
  totalSound += soundValue;
  soundCounter++;
  
  // Sample 30 second blocks
  if(((lastSoundMessage + 30000) <= milliCounter) || lastSoundMessage == 0) {
    avgSound = totalSound / soundCounter;
    if(maxSound >= avgSound) {
      Serial.print("Sound:");
      Serial.print(maxSound);
      Serial.println("");
    }
    // Reset average
    avgSound = soundValue;
    totalSound = soundValue;
    soundCounter = 0;
    lastSoundMessage = milliCounter;
    maxSound = soundValue;
  }
  

  tp401aSensor.read();
  
  if((lastAirMessage + 100000 < milliCounter) || lastAirMessage == 0) {
    lastAirMessage = milliCounter;
    airAvg = tp401aSensor.averageRead();
    if((lastAirRead > (airAvg + 5)) || (lastAirRead < (airAvg - 5))) {
      
      lastAirRead = tp401aSensor.lastRead();
      
      slcd.setCursor(15, 1);
      slcd.print("Air: ");
      slcd.print(lastAirRead, 10);
      Serial.print("Air:");
      Serial.print(lastAirRead);
      Serial.println("");
      
      tp401aSensor.reset();
    }
  }
     

 
  if(((lastBaromCheck + 40000) <= milliCounter) || lastBaromCheck == 0) {
    tempBaromTemp = barom.getTemperature();
    tempPress = barom.getPressure();

    if(savedBaromTemp != tempBaromTemp && (tempPress >= (savedBaromTemp + 0.1) || tempPress <= (savedBaromTemp - 0.1))) {
      savedBaromTemp = tempBaromTemp;
      slcd.setCursor(0,0);
      slcd.print("Temp: ");
      slcd.print(tempBaromTemp, 1);
      slcd.print("c");
      Serial.print("Temperature:");
      Serial.print(tempBaromTemp, DEC);
      Serial.println("");
    }
    
    // If pressure is different, and is +/- 20 different to the last reading, record it.
    if(savedPress != tempPress && (tempPress >= (savedPress + 20) || tempPress <= (savedPress - 20))) {
      savedPress = tempPress;
//      slcd.clear();
      slcd.setCursor(12,0);
      slcd.print("Pressure: ");
      slcd.print(float(tempPress), 1);
      slcd.print(" Pa");
      Serial.print("Pressure:");
      Serial.print(float(tempPress), DEC);
      Serial.println("");
    }
           
    lastBaromCheck = milliCounter;
  }
  
  if(((lastHumidCheck + 50000) <= milliCounter) || lastHumidCheck == 0) {

    dht11Sensor.readData(DHT11_PORT);
    if(savedHumid != dht11Sensor.humidity && (dht11Sensor.humidity >= (savedHumid + 1) || dht11Sensor.humidity <= (savedHumid - 1))) {
      slcd.setCursor(0,1);
      slcd.print("Humidity: ");
      slcd.print(float(dht11Sensor.humidity), 1);
      Serial.print("Humidity:");
      Serial.print(dht11Sensor.humidity, DEC);
      Serial.println("");
      savedHumid = dht11Sensor.humidity;
    }
    
    if(savedDewPoint != dht11Sensor.dewPointFast(dht11Sensor.temperature, dht11Sensor.humidity) && (dht11Sensor.dewPoint >= (savedDewPoint + 0.1) || dht11Sensor.humidity <= (savedDewPoint - 0.1))) {
      Serial.print("DewPoint:");
      Serial.print(dht11Sensor.dewPoint, DEC);
      Serial.println("");
      savedDewPoint = dht11Sensor.dewPoint;
    }
      
    lastHumidCheck = milliCounter;
  }
    
  
  if(((lastScroll + 300) <= milliCounter) || lastScroll == 0) {
    slcd.scrollDisplayLeft();
    lastScroll = milliCounter;
  }
  
}
