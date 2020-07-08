// ---------------------------------------------------------------------------
// Safety Library - v0.0.1 - 07/03/2020
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

//  This subsystem tries to help any rover-type robot be a little safer to work
//  with.  It does a few things:
//  1. Watchdog timer against companion computer.
//  2. Enables sensors to set a flag indicating motors should stop.
//  3. Enables end user to over-ride sensor safety triggers.

// It's really nothing more than a few simple booleans and access functions.

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

#include <Arduino.h>

#ifndef SAFE_ONCE
#define SAFE_ONCE

#define SAFETY_INTERVAL 1000000
class SafetyManager
{
public:
    SafetyManager(volatile uint32_t *tickCounter);
    bool IsSafe();
    bool IsConfigured(); // robot is configured or not yet?
    void SetConfigured(boolean value);
    void SetSensorTrigger(boolean value);
    void SetSafetyOverride(boolean value);
    void Reset();
    void Dispatch();
    String ResetWatchDog();

private:
    boolean m_sensorTriggered; // did a sensor trigger a safety problem?
    boolean m_userOverride; // did the user request an override of the sensor system?
    volatile uint32_t *m_tickCounter;
    uint32_t m_watcdogLastTick; // When was the watchdog last reset?
    boolean m_watchdogFired; // did the watchdog fire a timeout?
    uint32_t m_watchDogRequestcount;
    boolean m_IsConfigured;
};
#endif