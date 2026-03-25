#pragma once
#include <string>
#include "include/serial/SerialPort.hpp"
#include "include/json.hpp"

using json = nlohmann::json;

struct ArduinoBaseData {
    float pos = 0.0f;
    float gas = 0.0f;   // pot1
    float brake = 0.0f;   // pot2
};

struct ArduinoWheelData {
    int   enc1 = 0;
    int   enc2 = 0;
    float accelX = 0.0f;
    float accelY = 0.0f;
    float accelZ = 0.0f;
    bool  switchTL = false;
    bool  switchTR = false;
    bool  switchBL = false;
    bool  switchBR = false;
    int   joyDir = 0;
};

class ArduinoManager {
public:
    ArduinoManager();
    ~ArduinoManager();

    bool connectBase(const std::string& port);  // ex: "\\\\.\\COM3"
    bool connectWheel(const std::string& port);  // ex: "\\\\.\\COM4"

    void update();  // à appeler dans gameLoop()

    void sendToWheel(float rpm, float maxRpm, int gear,
        float fuel, float tireWear,
        bool inPit, float speed);

    ArduinoBaseData  getBaseData()  const { return baseData; }
    ArduinoWheelData getWheelData() const { return wheelData; }

private:
    SerialPort* basePort = nullptr;
    SerialPort* wheelPort = nullptr;

    ArduinoBaseData  baseData;
    ArduinoWheelData wheelData;

    std::string baseBuffer;
    std::string wheelBuffer;

    bool RcvFromSerial(SerialPort* port, std::string& msg);
    bool SendToSerial(SerialPort* port, json j_msg);

    void parseBase(const std::string& raw);
    void parseWheel(const std::string& raw);
};

