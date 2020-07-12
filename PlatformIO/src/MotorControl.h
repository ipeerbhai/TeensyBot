// ---------------------------------------------------------------------------
// Motor Control Library - v0.0.1 - 07/03/2020
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

//  Both steppers and servo motors can be thought of as timed signals with 1 micosecond resolution.
//  Essentially, both motors are controlled by PWM signals of different frequencies.
//  For servos, there's a 20ms signal length, with the on-time of the 20ms signal determining angle of the servo.
//  For steppers, there's a step pulse sent to advance the motor, which can be though of as a PWM signal and duty cycle.
//  To complicate things, how many step pulses are sent matter for position control.
//
//  This library aims to create a universal "thread" that can control many motors at the same time.
//  The motors are all configured as if they were steppers being controlled by PWM signals.
//  It works by using tick-counting, and presumes a real-time tick-count and 1uS tick resolution.

//  The library uses GPIO to simulate PWM.  No need to use a PWM enabled pin.
//  The library has been tested on the Sparkfun Artemis ATP, and should work on anything faster.

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

#include <Arduino.h>
#include <stdint.h>
#include "SafetySystem.h"

#ifndef MOTOR_ONCE
#define MOTOR_ONCE

typedef struct 
{
    int8_t EnablePin;
    int8_t DirPin;
    int8_t PulsePin;
    uint8_t PulsePrevState;
    uint8_t PulseDesiredState;
    uint32_t Interval;
    uint32_t DutyInterval;
    uint32_t TicksLeft;
} MotorController;

class MotorControl
{
public:
    MotorControl();
    MotorControl(int howMany, volatile uint32_t *tickCounter, uint32_t *prevTickCounter, SafetyManager *safetyPtr);
    void Init(int howMany, volatile uint32_t *tickCounter, uint32_t *prevTickCounter, SafetyManager *safetyPtr);
    void ConfigureMotor(int motorIndex, int8_t enablePin, int8_t dirPin, int8_t pulsePin, uint32_t interval, uint32_t dutyInterval);
    void Dispatch();
    void SafeDigitalWrite(int pin, int level);
    void UpdateMotorTimings(int idx, String command);
    void SetMotorState(int motorId, int state);
    void StopMotors();

private:
    MotorController m_motors[9];
    uint8_t m_motorCount;    // how many motors do we have? Set once, then don't change.
    int m_selectedMotor; // use this to iterate over the motors without doing an alloc.
    volatile uint32_t *m_tickCounter;
    uint32_t *m_prevTickCounter;
    SafetyManager *m_safetyManager; // to listen to the safety system
};

#endif