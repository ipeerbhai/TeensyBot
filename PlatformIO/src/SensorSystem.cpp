#include "SensorSystem.h"

SensorManager::SensorManager()
{
}

SensorManager::SensorManager(int howManyUS, volatile uint32_t *tickCount, SafetyManager *safetyPtr)
{
    Init(howManyUS, tickCount, safetyPtr);
}

void SensorManager::Init(int howManyUS, volatile uint32_t *tickCount, SafetyManager *safetyPtr)
{
    Serial.println("Initializing Sensor System");
    Serial.flush();

    m_safetyManager = safetyPtr; // so we can tell the sensor manager something's wrong.
    m_ultrasonicCount = howManyUS;
    m_tickCount = tickCount;

    Serial.println("Sensor system initialized");
    Serial.flush();
}

void SensorManager::ConfigureUltrasonic(int sensorIndex, uint8_t echoPin, uint8_t triggerPin, unsigned long maxDuration, unsigned long minDuration)
{
    if ((sensorIndex >= m_ultrasonicCount) || (sensorIndex < 0))
    {
        return; // do nothing, this sensor makes no sense.
    }
    m_ultrasonics[sensorIndex].EchoPin = echoPin;
    m_ultrasonics[sensorIndex].TriggerPin = triggerPin;
    m_ultrasonics[sensorIndex].MaxAllowedDurationUS = maxDuration;
    m_ultrasonics[sensorIndex].MinAllowedDurationUS = minDuration;
}

void SensorManager::ConfigureBattery(int pin)
{
    m_batteryPin = pin;
}

uint8_t SensorManager::GetBatteryLevel()
{
    uint8_t batLevel = (uint8_t)(m_batteryLevel / MAX_BATTERY_LEVEL) * 100;
    return (batLevel);
}

String SensorManager::ReadLatestUltrasonicState()
{
    String Text = String("");
    Text += String("{'Sensors': [");
    for (m_selectedSensor = 0; m_selectedSensor < m_ultrasonicCount; m_selectedSensor++)
    {
        Text += String("'");
        Text += String(m_selectedSensor);
        Text += String("':");
        Text += String(m_ultrasonics[m_selectedSensor].LastDurationUS);
        Text += String(",");
    }
    Text += String("]}");
    return (Text);
}

String SensorManager::ReadBatteryLevel()
{
    // switch to read mode, and then read it.
    String Text = String("");
    Text += String("{'Battery': [");
    for (int sampleNumber = 0; sampleNumber < BATTERY_SAMPLES; sampleNumber++)
    {
        int battery = analogRead(m_batteryPin);
        Text += String("'Sample");
        Text += String(sampleNumber);
        Text += String("':");
        Text += String(battery);
        Text += String(",");
    }
    Text += String("]}");
    return (Text);
}

//-----------------------------------------------------------------------------------------
// Dispatch iterates over all sensors ( ultrasonic and battery level ), and does whatever it takes to read them
void SensorManager::Dispatch()
{
    for (m_selectedSensor = 0; m_selectedSensor < m_ultrasonicCount; m_selectedSensor++)
    {
        // for this ultrasonic sensor, determine its phase and do the approporiate action.
        UltrasonicSensor *theSensor = &m_ultrasonics[m_selectedSensor];
        switch (theSensor->CurrentPhase)
        {
        case TRIGGER_OFF:
            // has it been long enough?
            if ((*m_tickCount - theSensor->PhaseChangeTimeUS) >= TRIGGER_OFF_TIME)
            {
                // phase change to trigger on
                theSensor->CurrentPhase = TRIGGER_ON;
                pinMode(theSensor->TriggerPin, OUTPUT);
                digitalWrite(theSensor->TriggerPin, LOW);
            }
            break;

        case TRIGGER_ON:
            if ((*m_tickCount - theSensor->PhaseChangeTimeUS) >= TRIGGER_ON_TIME)
            {
                // trigger has been on for a while, phase change to listen.
                theSensor->CurrentPhase = LISTEN; // request a listen.
            }
            break;

        case LISTEN:
            // The hard part -- attach rise and fall interrupt ISRs to get the echo time.
            digitalWrite(theSensor->TriggerPin, LOW);
            pinMode(theSensor->EchoPin, INPUT);
            Serial.println("Not implimented yet");
            // TODO: Reset phase to trigger_off
            break;

        default:
            break;
        }
    }
}