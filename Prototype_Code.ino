//Include library for temperature and humidity sensor
//Declare pin for sensor and type of sensor
//Create a temperature and humidity sensor object
#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//Include library for soil moisture sensors and create two sensor objects
#include "Adafruit_seesaw.h"
Adafruit_seesaw ss1;
Adafruit_seesaw ss2;

//Decalre RGB LED pins
const int RED = 9;
const int GREEN = 10;
//Declare water level variables
int waterLow;
int waterLev;

//Declare pins for the ultrasonic rangefinder
//Declare variables for distance calculation
const int TRIG = 7;
const int ECHO = 8;
long duration;
int distance;

//Set up the LCD
#include <LiquidCrystal.h>
const int rs = 12, en = 11, d4 = 6, d5 = 5, d6 = 4, d7 = 3;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Declare pump pin and set watering duration to 5 sec
const int PUMP = 1;
const int WATER_TIME = 5 * 1000;
int dry;

//Set time between readings to 5 minutes
const int WAIT_TIME = (5*60) * 1000;

void setup() {
  // Declares pins 1-12 as output pins
  pinMode(RED , OUTPUT);
  pinMode(GREEN, OUTPUT);
  
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(PUMP, OUTPUT);

  //Begins temp & humidity sensor
  dht.begin();

  //Begins the soil moisture sensors
  ss1.begin(0x36);
  ss2.begin(0x37);

  //Start the LCD screen and set up the basic message format
  lcd.begin(16, 2);
  lcd.setCursor(8,0);
  lcd.write("Temp:");
  lcd.setCursor(15,0);
  lcd.print("F");
  lcd.setCursor(8,1);
  lcd.print("Humd:");
  lcd.setCursor(15,1);
  lcd.print("%");

  //Turn pump off
  digitalWrite(PUMP, LOW);
}

void ReadTempAndHum() {
  //Reads the temp&humidty from the sensor
  int h = dht.readHumidity();
  int f = dht.readTemperature(true);
  //Writes the temp&humidity to the LCD
  lcd.setCursor(13,0);
  lcd.print(" ");
  lcd.setCursor(13,0);
  lcd.print(f);
  lcd.setCursor(13,1);
  lcd.print(" ");
  lcd.setCursor(13,1);
  lcd.print(h);
}

int calcDistance() {
  //Turns off ultrasonic rangefinder
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  //Sends out a sound wave for 10 us, then turns URF off
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  //Finds how long it took for the sound wave to come back
  duration = pulseIn(ECHO, HIGH);
  //Calculates and returns the distance in cm
  //Uses the fact that the speed of sound is ~343,000 m/s, converts this to
  //0.0343 cm/us, multiplies by the time (us), and divides by 2 (accounting for
  //sound having to travel down and back)
  int dist = duration * 0.0343 / 2;
  return dist;
}

int checkWaterLevel(int dist_wat) {
  //Indicates that the water is low if the distance to the water level
  //is greater than 23 cm
  if (distance >= 23)
  {
    waterLow = 1;
  }
  else if(distance < 23)
  {
    waterLow = 0;
  }

  return waterLow;
}

void WaterLevelAction(int is_low) {
  //If the water isn't low, displays that fact to the LCD
  //Checks to see if the soil is dry
  if (waterLow == 0)
  {
    digitalWrite(GREEN, HIGH);
    digitalWrite(RED, LOW);
    lcd.setCursor(0,0);
    lcd.write("        ");
    lcd.setCursor(0,0);
    lcd.write("Water");
    lcd.setCursor(0,1);
    lcd.write(" ");
    lcd.setCursor(0,1);
    lcd.write("Full");
    isSoilDry();
  }
  //If the water is low, prints that to the LCD and does nothing else
  if (waterLow == 1)
  {
    digitalWrite(GREEN, LOW);
    digitalWrite(RED, HIGH);
    lcd.setCursor(0,0);
    lcd.write("        ");
    lcd.setCursor(0,0);
    lcd.write("Water");
    lcd.setCursor(0,1);
    lcd.write("    ");
    lcd.setCursor(0,1);
    lcd.write("Low");
  }
}

void isSoilDry(){
  //Reads the moisture content of the soil as a capacitive measurement
  uint16_t capread1 = ss1.touchRead(0);
  uint16_t capread2 = ss2.touchRead(0);

  //Calculates the average value returned by the 2 sensors
  int average = (capread1 + capread2) / 2;

  //If the average is less than 450, sets the soil to dry
  if (average <= 450)
  {
    dry = 1;
  }
  //If the average is > 450, sets the soil to not dry
  else if (average > 450)
  {
    dry = 0;
  }
  //If the soil is dry, waters the plants and sets the soil to not dry
  if (dry == 1)
  {
    waterPlants();
    dry = 0;
  }
}

void waterPlants() {
  //Displays "Watering" on the LCD and turns the pump on for 5 seconds
  lcd.setCursor(0,0);
  lcd.write("     ");
  lcd.setCursor(0,1);
  lcd.write("    ");
  lcd.setCursor(0,0);
  lcd.write("Watering");
  digitalWrite(PUMP, HIGH);
  delay(WATER_TIME);
  digitalWrite(PUMP, LOW);
}

void loop() {
  // Reads the temp&humidity, calculates the distance, checks the water level, and takes the appropriate action
  //Waits 5 minutes
  ReadTempAndHum();

  distance = calcDistance();

  waterLev = checkWaterLevel(distance);

  WaterLevelAction(waterLev);
  
  delay(WAIT_TIME);
}
