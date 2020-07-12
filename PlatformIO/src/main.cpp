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

MotorControl g_robotMotors;     // motor control subsystem
SafetyManager g_safetySystem;   // safety subsystem
SensorManager g_sensorSystem;   // sensor subsystem
CommandManager g_commandSystem; // Command/Control subsystem

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
  g_robotMotors.Dispatch();
  g_sensorSystem.Dispatch();
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
  // init serial port
  while (!Serial && millis() < 4000)
    Serial.begin(115200);
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  g_safetySystem.Init(&g_TimerCounter);
  g_commandSystem.Init(&g_robotMotors, &g_TimerCounter, &g_PrevTimerCounter, &g_sensorSystem, &g_safetySystem);
  Serial.println("Ready>");
  RequestConfiguration();

  // Wait until we get a configuration before we do the rest
//  uint32_t x = 0;
  while (!g_safetySystem.IsConfigured())
  {
    g_commandSystem.Dispatch();
    delay(200);
//    Serial.print("Setup Loop ");
//    Serial.println(x);
//    x++;
  }

  // start the timer.
  Serial.println("Starting dispatch system");
  Serial.print("Motor dispatch ptr::");
  g_mainTimer.begin(Dispatch, 1);
}

void loop()
{
  // Run the non-critical dispatch functions.
  g_safetySystem.Dispatch();
  g_commandSystem.Dispatch();
}
