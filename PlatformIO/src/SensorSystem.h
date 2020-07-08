// ---------------------------------------------------------------------------
// Sensor Library - v0.0.1 - 07/03/2020
//
// AUTHOR/LICENSE:
// Created by Imran Peerbhai -- ipeerbhai@aol.com
// Copyright 2020 License:
// Forks and derivitive works are NOT permitted without
// permission. Permission is only granted to use as-is for private and
// non-commercial use by natural persons or non-profit entities.
// For-profit entities require a license.
//
// CONTRIBUTIONS:
// If you wish to contribute, make changes, or enhancements,
// please create a pull request.
//
// DISCLAIMER:
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Theory of Operation:

//  Sensors are slow and eat pins for breakfast.  The UC-025 sensors tested
//  take 85ms to acquire distance.  That's forever in mico-controller time.
//  Each UC-025 takes 4 pins -- 2 of which are signal (Trigger, Echo ).
//  We want to use only 1 signal pin per sensor, and we want to not wait around for the 85ms per sensor.

//  To achieve this, we set up a phase system that carefully monitors the single pin and changes how we use it.
//  We also create ISRs and enable them when the phase is correct for reading the echo.

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

#include <Arduino.h>
#include "SafetySystem.h"

#ifndef SENSOR_ONCE
#define SENSOR_ONCE

#define BATTERY_SAMPLES 5
#define MAX_BATTERY_LEVEL 1024

#define ULTRASONIC_TIMEOUT 1000000
#define TRIGGER_OFF_TIME 10000
#define TRIGGER_ON_TIME 50


enum UltrasonicSensorPhases
{
  TRIGGER_OFF,
  TRIGGER_ON,
  LISTEN
};

struct UltrasonicSensor
{
  // Pins and pads.
  uint8_t EchoPin;
  uint8_t TriggerPin;
  uint8_t EchoPad; // for sparkfun artemis.  Unsure if used in Teensy 4.1

  // Assistant variables like trackers, timers, etc
  UltrasonicSensorPhases CurrentPhase; // What's the current sensor phase?
  uint8_t StateFilter;                 // What pin state should we filter for when reading a return pulse?
  unsigned long PhaseChangeTimeUS;     // when did we change to this phase in microseconds?
  unsigned long LastDurationUS;        // how long was the last read duration ( use to compute distance )
  unsigned long MaxAllowedDurationUS;  // For safety, what will I allow before I say kaput.
  unsigned long MinAllowedDurationUS;  // For safety, what will the minimum I allow before I require over-ride?
};


class SensorManager
{
public:
    SensorManager(int howManyUS, volatile uint32_t *tickCount, SafetyManager *safetyPtr);
    void ConfigureUltrasonic(int sensorIndex, uint8_t echoPin, uint8_t TriggerPin, unsigned long maxDuration, unsigned long minDuration);
    void ConfigureBattery(int pin); // what analog pin is the battery voltage divider attached to?
    uint8_t GetBatteryLevel(); // returns a best-guess representing percent 0..100
    String ReadBatteryLevel(); // returns a JSON object with a number of samples
    String ReadLatestUltrasonicState(); // returns a JSON object with the last processed state of every sensor.
    void Dispatch(); // actually run the sensors and update the state machine.
private:
    UltrasonicSensor *m_ultrasonics; // a ptr to the array of ultrasonic sensors.
    int m_ultrasonicCount; // how many do we have attached to robot?
    int m_selectedSensor; // use for iterating or working with an individual ultrasonic sensor.
    int m_batteryPin; // use for reading the battery level.
    uint32_t m_batteryLevel; // What's the best-guess battery level?
    SafetyManager *m_safetyManager;
    volatile uint32_t *m_tickCount;
};

#endif