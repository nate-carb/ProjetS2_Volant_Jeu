#pragma once
#include <string>
#include <QObject>
#include <QTimer>
#include "include/serial/SerialPort.hpp"
#include "include/json.hpp"

using json = nlohmann::json;

struct ArduinoBaseData {
    float pos = 0.0f;
    float gas = 0.0f;
    float brake = 0.0f;
};

struct ArduinoWheelData {
    int   enc1 = 0;
    int   enc2 = 0;
    float accelX = 0.0f, accelY = 0.0f, accelZ = 0.0f;
    bool  switchTL = false, switchTR = false, switchBL = false, switchBR = false;
    int   joyDir = 0;
    bool  paddleshiftup = false;
    bool  paddleshiftdown = false;
    bool  prevPaddleUp = false;
    bool  prevPaddleDown = false;
};

class ArduinoManager : public QObject {
    Q_OBJECT
public:
    ArduinoManager(QObject* parent = nullptr);
    ~ArduinoManager();

    bool hasNewWheelData() {
        bool v = newWheelData;
        newWheelData = false;
        return v;
    }

    bool connectBase(const std::string& port);
    bool connectWheel(const std::string& port);

    void sendToWheel(float rpm, float maxRpm, int gear,
        float fuel, float tireWear,
        bool inPit, float speed, float angle);

    ArduinoBaseData  getBaseData()  const { return baseData; }
    ArduinoWheelData getWheelData() const { return wheelData; }

    bool isBaseReady()  const { return baseReady; }
    bool isWheelReady() const { return wheelReady; }

    bool prevPaddleUp = false;
    bool prevPaddleDown = false;
    bool prevtl = false, prevtr = false, prevbl = false, prevbr = false;
    bool prevEnc1 = false, prevEnc2 = false;

private slots:
    void update();   // appelé automatiquement par pollTimer

private:
    bool newWheelData = false;
    QTimer* pollTimer = nullptr;

    SerialPort* basePort = nullptr;
    SerialPort* wheelPort = nullptr;
    bool baseReady = false;
    bool wheelReady = false;

    ArduinoBaseData  baseData;
    ArduinoWheelData wheelData;

    std::string baseBuffer;
    std::string wheelBuffer;

    bool SendToSerial(SerialPort* port, json j_msg);
    void parseBase(const std::string& raw);
    void parseWheel(const std::string& raw);
};