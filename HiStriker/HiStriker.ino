#include <DmxMaster.h>

/********** VARIABLES **********/

// debug
//unsigned long cycleTimer = 0;   // timestamp at the start of the cycle
//int roundCount = 0;             // number of rounds played during uptime

// air pressure sensor
const int sensorPin = A0;       // input pin for the pressure sensor
int sensorValue = 0;            // current value from the sensor
int sensorMin = 0;              // minimum, baseline value coming from the sensor
int sensorMax = 0;              // maximum value coming from the sensor
const int sensorThreshold = 5;  // the sensor value noise threshold

// running average (to capture sensor idle/min value)
const int avgNumReadings = 6;   // number of values to sample
int avgReadings[avgNumReadings];// the readings from the analog input
int avgIndex = 0;               // the index of the current reading
int avgTotal = 0;               // the running total

// difficulty knob
const int potPin = A2;          // input pin for the potentiometer
int potValue = 0;               // current value from the potentiometer

// (internal) "round" (game is running)
boolean roundRunning = false;   // flag to store the start of the round (detected)
unsigned long roundTimer = 0;   // round timeout "watchdog"
int roundValue = 0;             // current score value of the round
int roundScore = 0;             // score of the round
int roundMax = 0;               // "winning" value of the round (adjusted by potentiometer)

// (internal) "idle" (game is waiting)
int idleValue = 0;              // idle light value
unsigned long idleTimer = 0;    // idle light timer

/********** INITIALIZE **********/

void setup() {
  //Serial.begin(9600);                 // debug interface

  //lightSetup();                       // initialize the lights

  sensorMin = analogRead(sensorPin);  // set initial, baseline value of air pressure sensor
  avgSetup(sensorMin);                // initialize moving average
  sensorMax = sensorMin + 50;         // set initial, maximum value of air pressure sensor
}

/********** MAIN PROGRAM **********/

void loop() {
  // debug - record time at start of cycle
  //cycleTimer = millis();

  // read the value from the air pressure sensor
  // minimum value updated during idle sequence, maximum updated here
  sensorValue = analogRead(sensorPin);
  if (sensorValue > sensorMax) sensorMax = sensorValue;

  // read the value from the difficulty knob potentiometer
  // map potentiometer value to difficulty
  potValue = analogRead(potPin);
  roundMax = map(potValue,0,1023,sensorMin+sensorThreshold,sensorMax+sensorMax/3);

  // STRIKER HAS BEEN HIT
  // if the game is idle and the sensor detects an increase beyond the noise threshold
  if (!roundRunning && sensorValue > sensorMin + sensorThreshold) {
    //roundCount++;           // debug
    roundTimer = millis();  // set round timer
    roundRunning = true;    // set round running flag
  }

  // ROUND ACTIVE
  // record & display high score
  if (roundRunning) {
    roundValue = map(sensorValue,sensorMin,roundMax,1,6);  // map sensor value to 6 lights (no "zero" score)
    if (roundValue > roundScore) roundScore = roundValue;  // the round score is the maximum value recorded
    lightSet(roundScore); // show the score
  }  else {
    roundValue = 0;
    roundScore = 0;
  }

  // ROUND FINISHED
  // if the round is active and the timer expired (2s)
  if (roundRunning && millis() > roundTimer + 2000) {
    // flash the round score 4 times
    for (int i = 0; i < 4; i++) {
      lightSet(0);          // all lights off
      delay(500);
      lightSet(roundScore); // lights on with score
      delay(1000);
    }
    roundTimer = 0;         // clear round timer
    roundRunning = false;   // clear round running flag
  }

  // IDLE
  // display an idle pattern
  if (!roundRunning) {
    if (millis() > idleTimer + 500) {
      // update lights
      lightSet(idleValue);
      idleValue++;
      if (idleValue > 6) idleValue = 0;

      // reset idle timer
      idleTimer = millis();

      // refresh sensor baseline value
      sensorMin = avgValue(sensorValue);
    }
    roundRunning = false;
  } else {
    idleTimer = 0;
    idleValue = 0;
  }

  // DEBUG
  /*if (roundRunning) {
    Serial.print("PLAY");
  } else {
    Serial.print("IDLE");
  }

  Serial.print(" Sensor: ");
  Serial.print(sensorMin,DEC);
  Serial.print("<");
  Serial.print(sensorValue,DEC);
  Serial.print("<");
  Serial.print(sensorMax,DEC);

  Serial.print(" Round ");
  Serial.print(roundCount,DEC);
  Serial.print(": ");
  Serial.print(roundValue,DEC);
  Serial.print("<");
  Serial.print(roundScore,DEC);
  Serial.print("<");
  Serial.print(roundMax,DEC);

  Serial.print(" Cycle: ");
  Serial.println(millis()-cycleTimer,DEC);*/
  // END DEBUG
}

/********** RUNNING AVERAGE **********/

void avgSetup(int value) {
  // initialize all the readings to 0:
  for (int i = 0; i < avgNumReadings; i++) {
    avgReadings[i] = value;
    avgTotal += value;
  }
}

int avgValue(int value) {
  avgTotal -= avgReadings[avgIndex];  // subtract the last reading
  avgReadings[avgIndex] = value;      // read from the sensor
  avgTotal += avgReadings[avgIndex];  // add the reading to the total
  avgIndex++;  // advance to the next position in the array

  // if we're at the end of the array, wrap around to the beginning
  if (avgIndex >= avgNumReadings) avgIndex = 0;

  // calculate the average
  return (avgTotal / avgNumReadings);
}

/********** LIGHTING **********/

// initialize lighting
void lightSetup() {
  // RGB lights
  //DmxMaster.usePin(3);
  //DmxMaster.maxChannel(6);
}

// set lighting (always cumulative)
void lightSet(int value) {
  // RGB Lights
  if (value > 0) {
    DmxMaster.write(11,128);  DmxMaster.write(12,130);  DmxMaster.write(13,  0);
  } else {
    DmxMaster.write(11,  0);  DmxMaster.write(12,  0);  DmxMaster.write(13,  0);
  }
  if (value > 1) {
    DmxMaster.write(14,154);  DmxMaster.write(15,112);  DmxMaster.write(16,  0);
  } else {
    DmxMaster.write(14,  0);  DmxMaster.write(15,  0);  DmxMaster.write(16,  0);
  }
  if (value > 2) {
    DmxMaster.write(17,180);  DmxMaster.write(18, 94);  DmxMaster.write(19,  0);
  } else {
    DmxMaster.write(17,  0);  DmxMaster.write(18,  0);  DmxMaster.write(19,  0);
  }
  if (value > 3) {
    DmxMaster.write(20,206);  DmxMaster.write(21, 76);  DmxMaster.write(22,  0);
  } else {
    DmxMaster.write(20,  0);  DmxMaster.write(21,  0);  DmxMaster.write(22,  0);
  }
  if (value > 4) {
    DmxMaster.write(23,232);  DmxMaster.write(24, 58);  DmxMaster.write(25,  0);
  } else {
    DmxMaster.write(23,  0);  DmxMaster.write(24,  0);  DmxMaster.write(25,  0);
  }
  if (value > 5) {
    DmxMaster.write(26,255);  DmxMaster.write(27, 40);  DmxMaster.write(28,  0);
  } else {
    DmxMaster.write(26,  0);  DmxMaster.write(27,  0);  DmxMaster.write(28,  0);
  }
}

