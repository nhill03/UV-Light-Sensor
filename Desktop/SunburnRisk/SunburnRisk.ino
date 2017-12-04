/*
  Noah Hill
  BME50 "Hack-A-Thon"
  December 2017
*/

/* 
   This is an Arduino sketch that determines a user's risk of 
   sunburn based on the UV-B intensity. The algorithm takes a
   voltage measurement every 25 seconds. 50 voltage measurements
   are taken at each 25 second interval, then averaged and returned
   as the instantaneous voltage. This takes 5 seconds; 5 + 25 = 30 
   seconds total. The algorithm keeps a running list of the data
   taken over the most recent 10 minutes, so 20 total nodes max.
*/

// define libraries
#include <Adafruit_NeoPixel.h>
#include <LinkedList.h>
#include <math.h>

// define the pins for arduino, and constant variables
#define PIN 2
#define n 0
#define inPin0 0

const double R1 = 2700.0;
const double R2 = 820000.0;
const double Rf = 820000.0;
const double GAIN = (1+(R2/R1));
const double SLOPE = 0.00000000166667;
// array of times-to-burn for each UV value
double BURNTIME[11] = {60.0,60.0,60.0,45.0,45.0,30.0,30.0,15.0,15.0,15.0,10.0};

//define variables for calculations
double current;
double volts1;
double volts2;
double averageTime;
int uvIndex;

// define list of UV index burn-times and linked list for data
LinkedList<int> runningList;
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);

// ***functions for informing risk via a Neopixel***

// Functions for pixel colors, turns it different colors
void green(void){
    pixel.setPixelColor(0, 0, 153, 0);
    pixel.show();
}
void yellow(void){
    pixel.setPixelColor(0, 255, 255, 0);
    pixel.show();  
}
void red(void){
    pixel.setPixelColor(0, 255, 0, 0);
    pixel.show();  
}
void blinkRed(void){
    pixel.setPixelColor(0,255,0,0);
    pixel.show();
    delay(1000);
    pixel.setPixelColor(0,0,0,0);
    pixel.show();
    delay(1000);
}
void off(void){
  pixel.setPixelColor(0,0,0,0);
  pixel.show();
}

// Blink the pixel different colors based on average time to burn
// ** will not turn a color if average time is 55+ **
void informRisk(double averageTime) {
  if(averageTime < 15)
    blinkRed();
  else if (averageTime < 30)
    red();
  else if(averageTime < 45)
    yellow();
  else if (averageTime < 55)
    green();
}

// Performs this light sequence on boot
void powerSequence(void) {
  for(int i = 0; i < 4; i++) { 
    green();
    delay(500);
    off();
    delay(500);
  }
  green();
  delay(1000);
  off();
  delay(125);
  pixel.setPixelColor(0,0,255,255);
  pixel.show();
  delay(125);
  off();
  delay(125);
  pixel.setPixelColor(0, 0, 128, 255);
  pixel.show();
  delay(125);
  off(); 
  delay(125);
  pixel.setPixelColor(0,0,0,255);
  pixel.show();
  delay(125);
  off();
  delay(125);
  pixel.setPixelColor(0, 255, 125, 255);
  pixel.show();
  delay(125);
  off(); 
  delay(125);
  pixel.setPixelColor(0,255,51,153);
  pixel.show();
  delay(125);
  off();
  delay(125);
  red();
  delay(2000);
  off();
}

// ***Functions for getting voltages and current of circuit***

// Reads the voltage across ground and analog pin 0 on arduino (2nd amp)
// analog pin 0 gives integer between 0-1024, linear relationship to voltage
// divide by 1024.0 to get fraction, then multiply by 5.0 volts to get voltage.
// Calculates the average voltage reading over 5 seconds of readings.
// @return sumavg = avg voltage measured from the second amplifier, from pin
double getVoltage2(void) {
  double sumavg = 0; //calculate avg voltage over 5 seconds, beat noise
  for(int i = 0; i<50; i++) {
    // take voltage reading 50 times over 5 seconds, 100ms intervals
    int pinRead0 = analogRead(inPin0);
    double pVolt0 = pinRead0 / 1024.0 * 5.0;
    sumavg += pVolt0;
    delay(100);
  }
  sumavg = sumavg / 50.0;
  return sumavg;
}

// Determines the original photodiode current
// volts2 = volts1*r = volts1(1+R1/R2) = volts1*gain 
// volts1 = i*Rf, i = photodiode current ... i = V2/(Rf*gain)
// @return i = original photodiode current
double getCurrent(double volts2) {
  double i;
  i = volts2/(Rf*GAIN);
  return i;
}

// Determines the UV Index based on the current from the photodiode and 
// data sheet graph, linear relationship UV vs. photodiode current
// Rounds UV index up/down to nearest UV index (i.e UV 2.6 = UV 3)
// @return index = UV index
int getIndex(double current) {
  int index = current/SLOPE;
  return index;
}

// adds values normally when size < 15
// when size is greater than 15, deletes the oldest (shift() function) and adds new data
void addValueToList(double timeToBurn) {
  // when runningList.size() < 15, add normally
  if(runningList.size() < 20)
    runningList.add(timeToBurn);
  // when runningList.Size() = 20, remove oldest data and add new data to end
  else {
    runningList.shift();
    runningList.add(timeToBurn);
  }
}

// Calculates the average of all linked list values 
// @return average = the average of all values in the linked list
double getAverage(void) {
  double sum = 0.0;
  double average = 0.0;
  // compute the average of the runningList
  for(int i = 0; i < runningList.size(); i++){
    sum += runningList.get(i);
  }
  double cast = (double) runningList.size();
  average = sum/cast;
  return average;
}

// ***setup and loop***

// Setup initalizes the pixels so they can be used to inform burn risk
// Performs the power sequence so user can make sure the board is on
void setup(void) {
  // initialize pixels, set brightness to not bright asf
  pixel.begin();
  pixel.setBrightness(75);
  pixel.show();
  powerSequence();
}

// meassures voltage, calculates current, calculates UV index,
// weighs UV index against appr. time until sunburn occurs,
// stores most recent 10 mins of data in linked list.
// Sunburn risk assessed on the average of the linked list risk values
void loop(void) {
  volts2 = getVoltage2();
  current = getCurrent(volts2);
  uvIndex = getIndex(current);
  //adds time to burn accoring to UV index to the running list;
  //add function will remove oldest value when list reaches 15, then
  //add most recent value to keep a "running list" of last 15 values
  double burntime = BURNTIME[uvIndex];
  addValueToList(burntime);
  averageTime = getAverage();
  informRisk(averageTime);
  delay(25000); // 25 seconds + 5 seconds to take avg. voltage = 30 seconds
}
