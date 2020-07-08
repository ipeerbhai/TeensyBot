// ---------------------------------------------------------------------------
// Communications Library - v0.0.1 - 07/03/2020
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

//  We receive commands from a main computer, which we then execute.
//  An example command is something like a motor timing change.
//  commands start with a letter like "a", have the parameters, and end with "~"
//  So a command looks like "m+00000~"

// Motors are difficult, because we don't know ahead of time how many we have.
// So, we require motors commands in pairs.
//  m01+00000,02-00000~ with 00 being an ignore m00+00000 is "ignore this"

#include <Arduino.h>
#include "MotorControl.h"
#include "SensorSystem.h"
#include "SafetySystem.h"

#ifndef COMMAND_ONCE
#define COMMAND_ONCE

#define SUBSTRINGS_LIMIT 10

class CommandManager
{
public:
    CommandManager(MotorControl *motorSystem, volatile uint32_t *tickCounter, uint32_t *prevTickCounter, SensorManager *sensorSystem, SafetyManager *safetySystem);
    void ProcessCommandBuffer();
    boolean ReadSerialPortData();
    void Dispatch();

private:
    String m_commandBuffer;         // for string handling
    MotorControl *m_motorControl;   // to hold the motor system
    SensorManager *m_sensorManager; // to talk with the sensor system
    SafetyManager *m_safetyManager; // to talk with the safety system
    volatile uint32_t *m_tickCounter;
    uint32_t *m_prevTickCounter;
};

#endif