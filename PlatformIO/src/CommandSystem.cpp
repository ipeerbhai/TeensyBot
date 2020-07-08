#include "CommandSystem.h"

CommandManager::CommandManager(MotorControl *motorSystem, volatile uint32_t *tickCounter, uint32_t *prevTickCounter, SensorManager *sensorSystem, SafetyManager *safetySystem)
{
    m_commandBuffer = String("");
    m_tickCounter = tickCounter;
    m_prevTickCounter = prevTickCounter;
}

//-----------------------------------------------------------------------------------------------------------------------------
// Procedure:
//  Read the command, execute the command, and send a response back.
// Expected command strings:
//  "b~" -- respond with battery analog reading (raw)
//  "c9,9~" -- we will configure 9 motors and 9 sensors.
//  "d0~" -- disable all steppers.
//  "d1~" -- disable stepper 1
//  "e0~" -- enable motor 0
//  "m+[5],-[5]~" -- set stepper 0, stepper 1 intervals to x and y
//  "o" -- override safety system checks.
//  "r" -- reset safety system and disable override.
//  "s~" -- read ultrasonic sensor and tell me the last duration.
//  "w~" -- let the watchdog know to reset.
//  "C~" -- configuration complete.
//  "M0,01,02,03,00000,00000~" -- configure motor 0 with enable pin 1, dir pin 2, pulse pin 3.
//  "M1,-1,-1,20000,00200~" -- configure motor 1 as a servo
//  "S0,01,01,700000,500~" -- configure sensor 0 with trigger and echo pin 01, 700,000 uS max allowed ping distance ( infinity) and 300uS min allowed ping distance (almost touching)
void CommandManager::ProcessCommandBuffer()
{
    // let's make a bunch of string object we can use to parse.
    String *subs = new String[SUBSTRINGS_LIMIT];
    for ( int i = 0; i < SUBSTRINGS_LIMIT; i++)
    {
        subs[i] = String("");
    }
    //   ResetWatchDog(); // we know the PC is alive, it just said something.
    String Text = String(""); // our return string.
    m_safetyManager->ResetWatchDog();
    switch (m_commandBuffer[0])
    {

    case 'b':
        // read and send back the battery analog level
        Text += m_sensorManager->ReadBatteryLevel();
        break;

    case 'c':
        // setup a bunch of motors and sensors.
        subs[0] += m_commandBuffer.substring(1, 2);
        subs[1] += m_commandBuffer.substring(3, 4);
        m_motorControl = new MotorControl(subs[0].toInt(), m_tickCounter, m_prevTickCounter, m_safetyManager);
        m_sensorManager = new SensorManager(subs[1].toInt(), m_tickCounter, m_safetyManager);
        Serial.println("Configured motors and sensors.");
        break;

    case 'd':
        // disable motors
        subs[0] += m_commandBuffer.substring(1, 2);
        m_motorControl->SetMotorState(subs[0].toInt(), LOW);
        Text += ("disabled " + subs[0]);
        break;

    case 'e':
        // enable motors
        subs[0] += m_commandBuffer.substring(1, 2);
        m_motorControl->SetMotorState(subs[0].toInt(), HIGH);
        Text += ("enabled " + subs[0]);
        break;

    case 'm':
        // Change motor speeds
        subs[0] += m_commandBuffer.substring(1, 7);
        subs[1] += m_commandBuffer.substring(8, 14);
        m_motorControl->UpdateMotorTimings(0, subs[0]);
        m_motorControl->UpdateMotorTimings(1, subs[1]);
        Text += String("motor strings::" + subs[0] + "::" + subs[1]);
        break;

    case 'o':
        // override safety system
        m_safetyManager->SetSafetyOverride(true);
        break;

    case 'r':
        // reset safety system.
        m_safetyManager->Reset();
        break;

    case 's':
        Text += m_sensorManager->ReadLatestUltrasonicState();
        break;

    // v0,200~ -- update servo 0 duty interval to 200uS.
    case 'v':
        // handle a ser(v)o duty interval update
        subs[0] += m_commandBuffer.substring(1,2); // which motor index to use?
        subs[1] += m_commandBuffer.substring(3); // 3 to end
        m_motorControl->UpdateMotorTimings(subs[0].toInt(), subs[1]);
        break;

    case 'w':
        Text += m_safetyManager->ResetWatchDog();
        break;

    //  "C~" -- configuration complete.
    case 'C':
        m_safetyManager->SetConfigured(true);
        break;

    //  "M0,01,02,03,00000,00000~" -- configure motor 0 with enable pin 1, dir pin 2, pulse pin 3.
    //  "M1,-1,-1,20000,000200~" -- configure motor 1 as a servo and set to 0 degrees.
    case 'M':
        subs[0] += m_commandBuffer.substring(1,2); // which motor?
        subs[1] += m_commandBuffer.substring(3,5); // enable pin ( or -1 for servos )
        subs[2] += m_commandBuffer.substring(6,8); // dir pin ( or -1 for servos )
        subs[3] += m_commandBuffer.substring(9,11); // pulse pin ( even for servos )
        subs[4] += m_commandBuffer.substring(12,17); // interval
        subs[5] += m_commandBuffer.substring(12,18); // duty interval
        m_motorControl->ConfigureMotor(subs[0].toInt(), (uint8_t)subs[1].toInt(), (uint8_t)subs[2].toInt(),(uint8_t)subs[3].toInt(),subs[4].toInt(), subs[5].toInt());
        break;

    //"S0,01,01,700000,500~"
    case 'S':
        subs[0] += m_commandBuffer.substring(1,2); // which sensor?
        subs[1] += m_commandBuffer.substring(3,5); // trigger pin
        subs[2] += m_commandBuffer.substring(6,8); // echo pin
        subs[3] += m_commandBuffer.substring(9,15); // max allowed duration
        subs[4] += m_commandBuffer.substring(16); // min allowed duration
        m_sensorManager->ConfigureUltrasonic(subs[0].toInt(), subs[2].toInt(), subs[3].toInt(), subs[4].toFloat(), subs[5].toFloat());
        break;
    default:
        Serial.println("{'Error' : 'Command not recognized'}");
        break;
    }
    Serial.println(Text);
}

//-----------------------------------------------------------------------------------------------------------------------------
// Function:
//  ReadSerialPortData keeps reading the port until it sees a ~ string.
boolean CommandManager::ReadSerialPortData()
{
    boolean returnCode = false;
    int serialBytesRecieved = 0;
    serialBytesRecieved = Serial.available();
    Serial.print("Received data bytes::");
    Serial.println(serialBytesRecieved);

    // we recieve data in fragments, but need to coalate it together.

    if (serialBytesRecieved > 0)
    {
        returnCode = true;
        Serial.println("ACK>");

        // we got a valid buffer but it's likely fragmented.
        m_commandBuffer += Serial.readStringUntil('~');
    }
    return (returnCode);
}

//-----------------------------------------------------------------------------------------------------------------------------
// Procedure:
//  Dispatch is the thing we call to actually process a command.
void CommandManager::Dispatch()
{
    if (ReadSerialPortData())
    {
        ProcessCommandBuffer();
    }
}