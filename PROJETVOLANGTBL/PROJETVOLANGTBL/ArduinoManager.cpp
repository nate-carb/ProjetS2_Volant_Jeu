#include "ArduinoManager.h"
#include <iostream>
#include <QDebug>
#define BAUD 115200
#define MSG_MAX 1024

ArduinoManager::ArduinoManager(QObject* parent) : QObject(parent) {

    pollTimer = new QTimer(this);
    connect(pollTimer, &QTimer::timeout, this, &ArduinoManager::update);
    pollTimer->start(20);  // 50Hz, toujours actif

}



ArduinoManager::~ArduinoManager() {

    delete basePort;
    delete wheelPort;

}



bool ArduinoManager::connectBase(const std::string& port) {

    baseReady = false;
    basePort = new SerialPort(port.c_str(), BAUD);
    if (!basePort->isConnected()) {

        std::cerr << "Base non connectee sur " << port << std::endl;

        delete basePort; basePort = nullptr;

        return false;

    }
    baseReady = true;
    return true;

}



bool ArduinoManager::connectWheel(const std::string& port) {

    wheelReady = false;

    wheelPort = new SerialPort(port.c_str(), BAUD);

    if (!wheelPort->isConnected()) {

        std::cerr << "Volant non connecte sur " << port << std::endl;

        delete wheelPort; wheelPort = nullptr;

        return false;

    }

    wheelReady = true;

    return true;

}



void ArduinoManager::update() {

    char char_buffer[1024];



    if (baseReady && basePort && basePort->isConnected()) {

        int size = basePort->readSerialPort(char_buffer, sizeof(char_buffer));

        if (size > 0) baseBuffer.append(char_buffer, size);

        size_t pos;

        while ((pos = baseBuffer.find('\n')) != std::string::npos) {

            std::string line = baseBuffer.substr(0, pos);

            baseBuffer = baseBuffer.substr(pos + 1);

            if (!line.empty() && line.back() == '\r') line.pop_back();

            if (!line.empty()) parseBase(line);

        }

    }



    if (wheelReady && wheelPort && wheelPort->isConnected()) {

        int size = wheelPort->readSerialPort(char_buffer, sizeof(char_buffer));

        if (size > 0) wheelBuffer.append(char_buffer, size);

        size_t pos;

        while ((pos = wheelBuffer.find('\n')) != std::string::npos) {

            std::string line = wheelBuffer.substr(0, pos);

            wheelBuffer = wheelBuffer.substr(pos + 1);

            if (!line.empty() && line.back() == '\r') line.pop_back();

            if (!line.empty()) parseWheel(line);

        }

    }

}



void ArduinoManager::sendToWheel(float rpm, float maxRpm, int gear,

    float fuel, float tireWear,

    bool inPit, float speed, float angle)

{

    if (!wheelReady || !wheelPort || !wheelPort->isConnected()) return;

    json j;
    j["r"] = rpm;
    j["rM"] = maxRpm;
    j["g"] = gear;
    j["f"] = fuel;
    j["t"] = tireWear;
    j["s"] = speed;
    j["p"] = inPit;
    j["a"] = angle;
    SendToSerial(wheelPort, j);

}


bool ArduinoManager::SendToSerial(SerialPort* port, json j_msg) {
    if (!port || !port->isConnected()) return false;
    std::string msg = j_msg.dump() + "\n";   
    bool ok = port->writeSerialPort(msg.c_str(), msg.length());
    if (!ok) {
        if (port == wheelPort) wheelReady = false;
        if (port == basePort)  baseReady = false;
    }
    return ok;
}

// ── Parsers JSON ─────────────────────────────────────────────────────────────
void ArduinoManager::parseBase(const std::string& raw)
{
    //qDebug() << "raw data" << QString::fromStdString(raw);
    try {
        json j = json::parse(raw);
        baseData.pos = j.value("pos", 0.0f);
        baseData.gas = j.value("pot1", 0.0f);
        baseData.brake = j.value("pot2", 0.0f);
    }
    catch (...) {
        std::cerr << "Erreur parse base: " << raw << std::endl;
    }
}

void ArduinoManager::parseWheel(const std::string& raw)
{
    try {
        json j = json::parse(raw);
        wheelData.enc1 = j.value("enc1", 0);
        wheelData.enc2 = j.value("enc2", 0);
        wheelData.accelX = j.value("accelX", 0.0f);
        wheelData.accelY = j.value("accelY", 0.0f);
        wheelData.accelZ = j.value("accelZ", 0.0f);
        wheelData.switchTL = j.value("switchTL", false);
        wheelData.switchTR = j.value("switchTR", false);
        wheelData.switchBL = j.value("switchBL", false);
        wheelData.switchBR = j.value("switchBR", false);
        wheelData.joyDir = j.value("JoyDirection", 0);
        wheelData.paddleshiftup = j.value("paddleshiftup", false);  
        wheelData.paddleshiftdown = j.value("paddleshiftdown", false);
        wheelData.muonCount = j.value("Muon", 0);
        newWheelData = true;
    }
    catch (...) {
        std::cerr << "Erreur parse volant: " << raw << std::endl;
    }
}