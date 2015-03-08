/*
 *  Vehicle Parking Alarm Via Ultrasonic Distance PING Sensor.
 *
 *  Copyright (C) 2010 Efstathios Chatzikyriakidis (contact@efxa.org)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// include notes' frequencies.
#include "pitches.h"

const int echoPin = 3;   // the PING sensor echo pin number (request).
const int trigPin = 2;   // the PING sensor trigger pin number (reply).
const int ledPin = 9;    // the pin number (PWM) of the led.
const int piezoPin = 10; // the pin number of the piezo.

const int SAMPLES_LENGTH = 10; // the number of PING sensor samples to fetch.
const long SAMPLE_DELAY = 100; // the delay in ms for next PING sensor sample.

long sensorMin = 0;   // start minimum PING sensor value.
long sensorMax = 100; // start maximum PING sensor value.

// PING sensor threshold value for the alarm.
const int sensorThreshold = 2000;

// serial data rate transmission: bps.
const long serialBaud = 9600;

// the current value of the led.
int ledValue;

// notes in the alarm melody.
const int notesMelody[] = { NOTE_C4, NOTE_G4, NOTE_C4, NOTE_G4 };

// calculate the number of the notes in the melody in the array.
const int NUM_NOTES = (int) (sizeof (notesMelody) / sizeof (const int));

// note durations: 4 = quarter note, 8 = eighth note, etc.
const int noteDurations[] = { 8, 4, 8, 4 };

// startup point entry (runs once).
void
setup() {
  pinMode(echoPin, OUTPUT); // set PING sensor echo as output.
  pinMode(trigPin, INPUT);  // set PING sensor trigger as input.

  // set the led and piezo as outputs.
  pinMode(ledPin, OUTPUT);
  pinMode(piezoPin, OUTPUT);

  // set serial data rate transmission.
  Serial.begin(serialBaud);
}

// loop the main sketch.
void
loop() {
  // get an average duration of back-and-forth ultrasonic PING signal.
  long dr = getPINGTime(trigPin, echoPin, SAMPLES_LENGTH, SAMPLE_DELAY);

  // if there was a normal duration time.
  if (dr >= 0) {
    // perform dynamic calibration on run-time.
    if (dr > sensorMax) sensorMax = dr;
    if (dr < sensorMin) sensorMin = dr;

    // map the duration time for the led (PWM).
    ledValue = map(dr, sensorMin, sensorMax, 0, 255);

    // check the value with the threshold value.
    if (dr <= sensorThreshold)
      // trigger the alarm melody.
      playMelody ();
  }
  else
    // negative duration times are abnormal.
    ledValue = 0;

  // write the value to the led.
  analogWrite(ledPin, ledValue);

  // calculate also the distance to inches and centimeters.
  long in = microsecondsToInches(dr);
  long cm = microsecondsToCentimeters(dr);

  // print to the serial the distance.
  Serial.print(in);
  Serial.print(" in, ");
  Serial.print(cm);
  Serial.print(" cm");
  Serial.println();
}

// convert duration time to distance in inches.
long
microsecondsToInches(const long ms) {
  return ms / 74 / 2;
}

// convert duration time to distance in centimeters.
long
microsecondsToCentimeters(const long ms) {
  return ms / 29 / 2;
}

// get an average duration time from the ultrasonic PING sensor.
long
getPINGTime(const int tPin, const int ePin, const int N, const long time) {
  static int curSample; // current sensor sample.
  static long curValue; // current sensor value.

  // current value variable first works as a sum counter.
  curValue = 0;

  // get sensor samples first with delay to calculate the sum of them.
  for (int i = 0; i < N; i++) {
    digitalWrite(ePin, HIGH);
    delayMicroseconds(10);
    digitalWrite(ePin, LOW);
    delayMicroseconds(2);

    // get sensor sample.
    curSample = pulseIn(tPin, HIGH);

    // add sample to the sum counter.
    curValue += curSample;

    // delay some time for the next sample.
    delay(time);
  }  

  // get the average sensor value.
  return (curValue / N);
}

// play a melody and return immediately.
void
playMelody() {
  // iterate over the notes of the melody.
  for (int thisNote = 0; thisNote < NUM_NOTES; thisNote++) {
    // to calculate the note duration, take one second divided by the note type.
    // e.g. quarter note = 1000/4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];

    // play the tone.
    tone(piezoPin, notesMelody[thisNote], noteDuration);

    // to distinguish notes, set a minimum time between them.
    // the note's duration plus 30% seems to work well enough.
    int pauseBetweenNotes = noteDuration * 1.30;

    // delay some time.
    delay(pauseBetweenNotes);
  }
}
