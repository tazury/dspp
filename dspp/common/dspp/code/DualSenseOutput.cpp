#include "DualSenseOutput.h"
#include "DualSense.h"
#include "Logger.h"

void DualSense::Output::Reset(bool useFlags)
{
    // Reset all members to default state

    if (bluetoothF && useFlags) { // Bluetooth controllers will reserve their Player Numbers so it doesn't look like it is off. - Disable this by making useFlags false
        uint8_t playerN = ledSettings.playerNumber;
        ledSettings = LedSettings();
        ledSettings.playerNumber = playerN;
    }
    else ledSettings = LedSettings();
    overallMotors = 0x00;
    leftMoter = 0x00;
    rightMoter = 0x00;
    leftTrigger = TriggerSettings();
    rightTrigger = TriggerSettings();
    sendOutput();
}

 uint8_t DualSense::Output::getLEDNumber()
{
    switch (controller.number) {
    case 1: return PlayerLED::Player1;
    case 2: return PlayerLED::Player2;
    case 3: return PlayerLED::Player3;
    case 4: return PlayerLED::Player4;
    default: return PlayerLED::None;  // No LED assigned
    }
}

int DualSense::Output::sendOutput()
{
    if (controller.is_usb) {
       return sendOutputUSB();
    }
    else return sendOutputBT();
}

int DualSense::Output::sendOutputUSB()
{ 
    while (controller.is_checking_connection) {
        // waiting
    }
  
    if (!controller.is_connected) return -2; // -1 Is a bad write | -2 is a bad connection
    std::array<uint8_t, 48> outputReport = { 0 }; // 48 Bytwes

    // USB mode implementation

        // Report header
    outputReport[0] = 0x02;  // Report ID for USB

    // Control flags (from C# example)
    outputReport[1] = 0xff; // 0x04 | 0x08;  // 0x4 | 0x8
    outputReport[2] = 0xff - 8;//0x01 | 0x04 | 0x10 | 0x40;

    outputReport[3] = leftMoter;  // Right motor (high frequency)
    outputReport[4] = rightMoter;   // Left motor (low frequency)

    if (ledSettings.muteLED) {
        outputReport[9] = 0x01;
        outputReport[10] = 0x00;
    }
    else {
        outputReport[9] = 0x00;
        outputReport[10] = 0x10;
    }

    // LED settings
    outputReport[39] = ledSettings.option;
    outputReport[43] = ledSettings.brightness;
    outputReport[42] = ledSettings.pulseOption;
    outputReport[44] = ledSettings.playerNumber;
    outputReport[45] = ledSettings.colors.red;
    outputReport[46] = ledSettings.colors.green;
    outputReport[47] = ledSettings.colors.blue;

    // Right trigger
    setTriggerUSB(outputReport, rightTrigger, 11);

    // Left trigger
    setTriggerUSB(outputReport, leftTrigger, 22);

    // Overall motors
    outputReport[38] = overallMotors;
    int returnBytes;
    try {
         returnBytes = hid_write(controller.hid_dev, outputReport.data(), outputReport.size());
    }
    catch (const std::exception& ex) {  // Catch by reference
        DS_LOG_ERROR(std::string("EXCEPTION OCCURRED ON OUTPUT WRITE USB: ") + ex.what());
    } 
    catch (...) {
        DS_LOG_ERROR("UNKNOWN EXCEPTION IN USB OUTPUT");
    }
    return returnBytes;
}

int DualSense::Output::sendOutputBT()
{
    while (controller.is_checking_connection) {
        // waiting
    }
    if (!controller.is_connected) return -2; // -1 Is a bad write | -2 is a bad connection
    std::array<uint8_t, 78> outputReportOG = { 0 }; // 78 Bytes for Bluetooth mode

    // Bluetooth mode implementation

    // Report Header (Bluetooth-specific)
   // outputReportOG[0] = 0x39;  // HID Bluetooth Set Report (0xA2)
    outputReportOG[0] = 0x31;
    outputReportOG[1] = static_cast<uint8_t>(outputSeq_ << 4);
    if (++outputSeq_ == 16)
        outputSeq_ = 0;

    // tag
    outputReportOG[2] = 0x10; // DS_OUTPUT_TAG


    std::array<uint8_t, 48> outputReport = { 0 }; // 48 Bytwes

    // USB mode implementation


    // Control flags (from C# example)

    outputReport[0] = 0xff; // 0x04 | 0x08;  // 0x4 | 0x8

    // On windows, bluetooth is a bit strange. I'll have to send an outputReport with id 0xff-7 than 0xff-9
    outputReport[1] = !bluetoothF ? 0xff - 1 : 0xff - 8;//0x01 | 0x04 | 0x10 | 0x40;

    outputReport[2] = leftMoter;  // Right motor (high frequency)
    outputReport[3] = rightMoter;   // Left motor (low frequency)

    if (ledSettings.muteLED) {
        outputReport[8] = 0x01;
        outputReport[9] = 0x00;
    }
    else {
        outputReport[8] = 0x00;
        outputReport[9] = 0x10;
    }

    // LED settings
    outputReport[39] = ledSettings.option;
    outputReport[42] = ledSettings.brightness;
    outputReport[41] = ledSettings.pulseOption;
    outputReport[43] = ledSettings.playerNumber;
    outputReport[44] = ledSettings.colors.red;
    outputReport[45] = ledSettings.colors.green;
    outputReport[46] = ledSettings.colors.blue;

    // Right trigger
    setTriggerUSB(outputReport, rightTrigger, 10);

    // Left trigger
    setTriggerUSB(outputReport, leftTrigger, 21);

    // Overall motors
    outputReport[37] = overallMotors;


    std::copy(outputReport.begin(), outputReport.end(), outputReportOG.begin() + 3);
    DualSenseChecksum::fillChecksum(0x31, outputReportOG);

    bluetoothF = true;

    int returnBytes;
    try {
        returnBytes = hid_write(controller.hid_dev, outputReportOG.data(), outputReportOG.size());
    }
    catch (const std::exception& ex) {  // Catch by reference
        DS_LOG_ERROR(std::string("EXCEPTION OCCURRED ON OUTPUT WRITE BT: ") + ex.what());
    }
    return returnBytes;
}

void DualSense::Output::setTriggerUSB(std::array<uint8_t, 47>& report, const TriggerSettings& trigger, int offset)
{
    report[offset] = trigger.mode;
    report[offset + 1] = trigger.force1;
    report[offset + 2] = trigger.force2;
    report[offset + 3] = trigger.force3;
    report[offset + 4] = trigger.force4;
    report[offset + 5] = trigger.force5;
    report[offset + 6] = trigger.force6;
    report[offset + 9] = trigger.force7;  // Note the gap in C# code

    // Handle GameCube mode special case
    if (trigger.mode == 8) {  // Assuming 8 is GameCube mode
        report[offset] = 2;   // Pulse mode
        report[offset + 1] = 0x90;
        report[offset + 2] = 0xA0;
        report[offset + 3] = 0xFF;
        // Zero out other parameters
        report[offset + 4] = 0;
        report[offset + 5] = 0;
        report[offset + 6] = 0;
        report[offset + 9] = 0;
    }
}

void DualSense::Output::setTriggerBT(std::array<uint8_t, 78>& report, const TriggerSettings& trigger, int offset)
{
    report[offset] = trigger.mode;
    report[offset + 1] = trigger.force1;
    report[offset + 2] = trigger.force2;
    report[offset + 3] = trigger.force3;
    report[offset + 4] = trigger.force4;
    report[offset + 5] = trigger.force5;
    report[offset + 6] = trigger.force6;
    report[offset + 9] = trigger.force7;  // Note the gap in C# code

    // Handle GameCube mode special case
    if (trigger.mode == 8) {  // Assuming 8 is GameCube mode
        report[offset] = 2;   // Pulse mode
        report[offset + 1] = 0x90;
        report[offset + 2] = 0xA0;
        report[offset + 3] = 0xFF;
        // Zero out other parameters
        report[offset + 4] = 0;
        report[offset + 5] = 0;
        report[offset + 6] = 0;
        report[offset + 9] = 0;
    }
}


void DualSense::Output::setTriggerUSB(std::array<uint8_t, 48>& report, const TriggerSettings& trigger, int offset)
{
    report[offset] = trigger.mode;
    report[offset + 1] = trigger.force1;
    report[offset + 2] = trigger.force2;
    report[offset + 3] = trigger.force3;
    report[offset + 4] = trigger.force4;
    report[offset + 5] = trigger.force5;
    report[offset + 6] = trigger.force6;
    report[offset + 9] = trigger.force7;  // Note the gap in C# code

    // Handle GameCube mode special case
    if (trigger.mode == 8) {  // Assuming 8 is GameCube mode
        report[offset] = 2;   // Pulse mode
        report[offset + 1] = 0x90;
        report[offset + 2] = 0xA0;
        report[offset + 3] = 0xFF;
        // Zero out other parameters
        report[offset + 4] = 0;
        report[offset + 5] = 0;
        report[offset + 6] = 0;
        report[offset + 9] = 0;
    }
}
