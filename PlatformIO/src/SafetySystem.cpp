#include "SafetySystem.h"

//-----------------------------------------------------------------------------------------
// Constructor:
//  Default constructor does nothing.
SafetyManager::SafetyManager()
{
}
//-----------------------------------------------------------------------------------------
// Constructor:
//  Reset the safety system and store a reference to the global tick counter.
SafetyManager::SafetyManager(volatile uint32_t *tickCounter)
{
    Init(tickCounter);
}

//-----------------------------------------------------------------------------------------
// Init initializes instance members.
void SafetyManager::Init(volatile uint32_t *tickCounter)
{
    m_tickCounter = tickCounter;
    m_watchDogRequestcount = 0;
    m_IsConfigured = false;
    Reset();
}

//-----------------------------------------------------------------------------------------
//  IsSafe verifies that it is still safe for motors to turn.
//  It will always honor the watchdog counter, but allows the user
//  to over-ride a sensor safety trigger.
bool SafetyManager::IsSafe()
{
    if (m_watchdogFired)
    {
        return (false);
    }
    if ((!m_sensorTriggered) || (m_userOverride))
    {
        return (true);
    }
    return (false);
}

//-----------------------------------------------------------------------------------------
//  IsConfigured verifies we got a valid configuration from the host PC.
bool SafetyManager::IsConfigured()
{
    return (m_IsConfigured);
}

//-----------------------------------------------------------------------------------------
//  SetConfigured is used by the communication manager to indicate a valid motor and sensor
//  configuration was received from the host PC.
void SafetyManager::SetConfigured(boolean value)
{
    m_IsConfigured = value;
}
//-----------------------------------------------------------------------------------------
// SetSensorTrigger allows the sensor manager to set safety flags.
void SafetyManager::SetSensorTrigger(boolean value)
{
    m_sensorTriggered = value;
}

//-----------------------------------------------------------------------------------------
// SetSafetyOverride allows the user (via communication manager) to over-ride a sensor trigger.
void SafetyManager::SetSafetyOverride(boolean value)
{
    m_userOverride = value;
}

//-----------------------------------------------------------------------------------------
// Reset fully resets the safety manager for sensors/communication triggers.
void SafetyManager::Reset()
{
    m_sensorTriggered = false;
    m_userOverride = false;
}

//-----------------------------------------------------------------------------------------
// Dispatch updates the watchdog timers, can fire a request to the main computer.
void SafetyManager::Dispatch()
{
    uint32_t computedInterval = m_watcdogLastTick - *m_tickCounter;
    if (computedInterval >= SAFETY_INTERVAL)
    {
        m_watchdogFired = true;
        if (m_watchDogRequestcount == 0)
        {
            // send a request to the main computer
            Serial.println("{'Request' : 'Watchdog'}");
            m_watchDogRequestcount++;
        }
    }
    else
    {
        m_watchdogFired = false;
        m_watchDogRequestcount = 0;
    }
}

//-----------------------------------------------------------------------------------------
// ResetWatchDog resets the watchdog trigger and tick counter.
// It returns a user-readable string of reset time.
String SafetyManager::ResetWatchDog()
{
    String Watchdog = String("");
    Watchdog += ("watchdog reset::");
    Watchdog += (*m_tickCounter);
    Watchdog += ("::");
    Watchdog += (m_watcdogLastTick);

    // reset the members needed.
    m_watchdogFired = false;
    m_watcdogLastTick = *m_tickCounter;

    return (Watchdog);
}