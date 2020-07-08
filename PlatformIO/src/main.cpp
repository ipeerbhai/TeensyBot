#include <Arduino.h>
#include <stdint.h>
#include "MotorControl.h"
#include "SafetySystem.h"
#include "SensorSystem.h"
#include "CommandSystem.h"

const int ledPin = 13; // for debugging.

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
// All the subsystems

MotorControl *g_RobotMotors; // motor control subsystem
SafetyManager *g_safetySystem; // safety subsystem
SensorManager *g_sensorSystem; // sensor subsystem
CommandManager *g_commandSystem; // Command/Control subsystem

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
// Create a dispatch system that guesses what pins need signal, then either changes pin
// stuate or returns

volatile uint32_t g_TimerCounter = 0; // used to count ticks, set by interrupt.
uint32_t g_PrevTimerCounter = 0;      // used to prevent double-call collisions.
IntervalTimer g_mainTimer;


//-----------------------------------------------------------------------------------------
// Dispatch is designed to run from within an interrupt.  It gets called every 1uS.
void Dispatch()
{
  // update the counter we want to use
  noInterrupts();
  g_TimerCounter++;
  interrupts();

  // Run critical Dispatch functions.
  g_RobotMotors->Dispatch();
  g_sensorSystem->Dispatch();
}



//-----------------------------------------------------------------------------------------
// Procedure:
//  Ask the main computer to send configuration commands.
void RequestConfiguration()
{
  Serial.println("{'Request' : 'Configuration'}");
}



//-----------------------------------------------------------------------------------------
// Procedure:
//  Standard Arduino setup.
void setup()
{
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  g_safetySystem = new SafetyManager(&g_TimerCounter);
  g_commandSystem = new CommandManager(g_RobotMotors, &g_TimerCounter, &g_PrevTimerCounter, g_sensorSystem, g_safetySystem);
  Serial.begin(500000);
  Serial.println("Ready>");
  RequestConfiguration();

  // Wait until we get a configuration before we do the rest
  while (!g_safetySystem->IsConfigured())
  {
    g_commandSystem->Dispatch();
    delay(200);
  } 
  
  // start the timer.
  g_mainTimer.begin(Dispatch, 1);
}

void loop()
{
  // Run the non-critical dispatch functions.
  g_safetySystem->Dispatch();
  g_commandSystem->Dispatch();
}