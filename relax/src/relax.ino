/*
 * Project Relax
 * Description:
 * Author:
 * Date:
 */

#include <math.h>

// These constants won't change. They're used to give names to the pins used:
const int analogInPin = A1;   // Analog input pin that the potentiometer is attached to
const int digitalOutPin = D7; // Analog output pin that vibration motor is attached to

int sensorValue = 0; // value read from the pot
int outputValue = 0; // value output to the PWM (analog out)

#define filter 6
#define filterAvg 10
double emg[filter];
double rollingAvg[filterAvg];
unsigned int count = 0;
unsigned int avgCount = 0;

int readDelta = 500;
int printDelta = 1000;
int vibrateDelta = 1000;
long readTime = 0;
long printTime = 0;
long vibrateTime = 0;
double prevAvg = 0;
double averageVal = 0;

//Particle
boolean publishStateNow = false;
boolean track = false;

String deviceID = "e00fce68058f85ccce40d570";
boolean tensed = false;

void setup()
{
  // initialize Serial communications at 9600 bps:
  Serial.begin(9600);
  pinMode(digitalOutPin, OUTPUT);   //Sets vibration motor as output
  digitalWrite(digitalOutPin, LOW); //No vibration at first
  //Particle
  Particle.function("publishState", publishState);
  Particle.function("trackSwitch", trackSwitch);
  Particle.variable("averageVal", averageVal);
  Particle.variable("tensed", tensed);
  Particle.variable("track", track);
}

void loop()
{
  long time = millis();
  if (time > vibrateTime)
  {
    digitalWrite(digitalOutPin, LOW); //Sets vibration motor to low
    vibrateTime += vibrateDelta;
    tensed = false;
  }
  if (time > readTime)
  {
    readVal();
    double std = stdDev(rollingAvg, filterAvg);
    if (std != 0 && averageVal > prevAvg + std)
    {
      tensed = true;
      Serial.print("std is ");
      Serial.print(std);
      Serial.print(". prevAvg+std = ");
      Serial.println(prevAvg + std);
      Serial.print(". current Avg = ");
      Serial.println(averageVal);
      Serial.println("RELAX!!");
      digitalWrite(digitalOutPin, HIGH); //Vibrates
      vibrateTime += vibrateDelta;       //Stops vibrating after vibrateDelta
    }
    readTime += readDelta;
  }
  if (time > printTime)
  { //Averaging value
    prevAvg = averageVal;
    averageVal = average(emg, filter);
    Serial.print("average value = ");
    Serial.println(averageVal);
    if (track)
    {
      //Only send data if tracking is turned on
      publishStateNow = true;
    }
    printTime += printDelta;
  }
  //Particle publishing state
  if (publishStateNow)
  {
    publishState("");
    publishStateNow = false;
  }
}
//Calculation functions
double average(double *inArray, int n)
{
  long sum = 0;
  for (int i = 0; i < n; i++)
  {
    sum += inArray[i];
  }
  return sum / n;
}
double stdDev(double *inArray, int n)
{
  double mean = average(inArray, n);
  long sum = 0;
  for (int i = 0; i < n; i++)
  {
    sum += pow(inArray[i] - mean, 2);
  }
  return sqrt(sum / n);
}
void readVal()
{ //Rolling value for input
  sensorValue = analogRead(analogInPin);
  outputValue = map(sensorValue, 0, 1023, 0, 100);
  emg[count % filter] = outputValue;
  count += 1;
  rollingAvg[avgCount % filterAvg] = averageVal; //Also record average values
  avgCount += 1;
}

//Particle
int publishState(String arg)
{
  // Goal: Publish a valid JSON string containing the state of the light:
  //   Ex:   "{"powered":true, "r":255, "g":255, "b":255}"
  //   Note that each property has double quotes!

  // TODO: Adjust variables to match your light's state variables
  // (NOTE: Be careful building the string.  C++ String operations often behave in
  //        counterintuitive ways when working with string literals (like "hi"))
  String data = "{";
  data += "\"averageVal\":\"";
  data += String(averageVal);
  data += "\", \"tensed\":\"";
  data += tensed;
  data += "\"}";

  Serial.println("Publishing:");
  Serial.println(data);

  Particle.publish("emg", data, 60, PRIVATE);
  return 0;
}
int trackSwitch(String arg)
{
  if (arg == "true") {
    track = true;
  } else{
    track = false;
  }
  // Goal: Publish a valid JSON string containing the state of the light:
  //   Ex:   "{"powered":true, "r":255, "g":255, "b":255}"
  //   Note that each property has double quotes!

  // TODO: Adjust variables to match your light's state variables
  // (NOTE: Be careful building the string.  C++ String operations often behave in
  //        counterintuitive ways when working with string literals (like "hi"))
  
  return 0;
}
void getPublishState()
{
  publishState("");
}