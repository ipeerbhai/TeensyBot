#include "MotorControl.h"

// --------------------------------------------------------------------------------------------------------------------
// Constructor:
//  MotorControl is a class that defines tick-counts needed to control different types of motors.
MotorControl::MotorControl(int howMany, volatile uint32_t *tickCounter, uint32_t *prevTickCounter, SafetyManager *safetyPtr)
{
    // create an array of motors
    m_motors = new MotorController[howMany];
    m_motorCount = howMany;
    m_tickCounter = tickCounter;
    m_prevTickCounter = prevTickCounter;
    m_selectedMotor = 0;

    // grab pointer to the global safety manager.
    m_safetyManager = safetyPtr;

}

// --------------------------------------------------------------------------------------------------------------------
// Procedure:
//  Configure a specific motor.
void MotorControl::ConfigureMotor(int motorIndex, int8_t enablePin, int8_t dirPin, int8_t pulsePin, uint32_t interval, uint32_t dutyInterval)
{
    // only run if the motor index is between 0 and motorcount -1
    if ((motorIndex < 0) || (motorIndex >= m_motorCount))
    {
        return;
    }
    m_motors[motorIndex].EnablePin = enablePin;
    m_motors[motorIndex].DirPin = dirPin;
    m_motors[motorIndex].PulsePin = pulsePin;
    m_motors[motorIndex].Interval = interval;
    m_motors[motorIndex].DutyInterval = dutyInterval;
}

// --------------------------------------------------------------------------------------------------------------------
// Procedure:
//  Actually set output pin levels based on time.  Call this from a 1 microsecond interrupt timer via a function pointer for performance.
void MotorControl::Dispatch()
{
    for (m_selectedMotor = 0; m_selectedMotor < m_motorCount; m_selectedMotor++)
    {
        m_motors[m_selectedMotor].TicksLeft = *m_tickCounter % m_motors[m_selectedMotor].Interval;

        // figure out if we've hit the cycle
        if (m_motors[m_selectedMotor].TicksLeft < m_motors[m_selectedMotor].DutyInterval)
        {
            m_motors[m_selectedMotor].PulseDesiredState = HIGH;
        }
        else
        {
            m_motors[m_selectedMotor].PulseDesiredState = LOW;
        }

        // change pin state and dispatch the pin if needed

        if (m_motors[m_selectedMotor].DutyInterval > 0)
        {
            // verify we're safe before we actually write...
            // but only if we're safe to do so.
            if (m_safetyManager->IsSafe())
            {
                digitalWrite(m_motors[m_selectedMotor].PulsePin, m_motors[m_selectedMotor].PulseDesiredState);
            }
        }
    }
}

// --------------------------------------------------------------------------------------------------------------------
// Procedure:
//  Only do a digital write to pins that are actually writable.  This allows me to abuse stepper logic for servos.
void MotorControl::SafeDigitalWrite(int pin, int level)
{
    if (pin >= 0)
    {
        digitalWrite(pin, level);
    }
}

// --------------------------------------------------------------------------------------------------------------------
// Procedure:
//  Update the timings the motor is using for dispatch.  Basically, re-configure the running motor.
void MotorControl::UpdateMotorTimings(int idx, String command)
{
    boolean isStepper = false;
    if (command[0] == '+')
    {
        // stepper, moving in + direction
        isStepper = true;
        SafeDigitalWrite(m_motors[idx].DirPin, HIGH);
    }
    else if (command[0] == '-')
    {
        // stepper, moving in - direction
        isStepper = true;
        SafeDigitalWrite(m_motors[idx].DirPin, LOW);
    }
    // extract the interval or dutyinterval
    int32_t someInterval = 0;

    if (isStepper)
    {
        // It's a stepper, we're getting an interval with 50% duty cycle.
        String intervalString = command.substring(1, command.length() - 1);
        someInterval = intervalString.toInt();
        m_motors[idx].Interval = someInterval;
        m_motors[idx].DutyInterval = someInterval / 2;
    }
    else
    {
        // It's a servo, with a fixed interval and we need to set the duty interval.
        someInterval = command.toInt();
        m_motors[idx].DutyInterval = someInterval;
    }
}

// --------------------------------------------------------------------------------------------------------------------
// Procedure:
//   Motors are not just PWM dispatches, but state machines.  A motor can be disabled, running, enabled and holding, etc...
void MotorControl::SetMotorState(int motorId, int state)
{
    if (motorId >= m_motorCount)
    {
        return; // do nothing, we don't have that motor.
    }
    m_motors[motorId].DutyInterval = 0;
    SafeDigitalWrite(m_motors[motorId].EnablePin, state);
}

// --------------------------------------------------------------------------------------------------------------------
// Procedure:
//  This sets all variabled to 0 to halt all motor signals.
void MotorControl::StopMotors()
{
  // set all duty intervals to 0, forcing a pull-down whenever the ISR fires.
  int motorCounter = 0;
  for (motorCounter = 0; motorCounter < m_motorCount; motorCounter++)
  {
    SetMotorState(motorCounter, LOW);
  }
}
