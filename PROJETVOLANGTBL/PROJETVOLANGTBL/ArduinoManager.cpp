#include "ArduinoManager.h"
#include <iostream>
#include <QDebug>
#define BAUD       9600
#define MSG_MAX    1024

ArduinoManager::ArduinoManager() {}

ArduinoManager::~ArduinoManager()
{
    delete basePort;
    delete wheelPort;
}

bool ArduinoManager::connectBase(const std::string& port)
{
    basePort = new SerialPort(port.c_str(), BAUD);
    if (!basePort->isConnected()) {
        std::cerr << "Erreur: impossible de connecter la base sur " << port << std::endl;
        return false;
    }
    std::cout << "Base connectee sur " << port << std::endl;
    return true;
}

bool ArduinoManager::connectWheel(const std::string& port)
{
    wheelPort = new SerialPort(port.c_str(), BAUD);
    if (!wheelPort->isConnected()) {
        std::cerr << "Erreur: impossible de connecter le volant sur " << port << std::endl;
        return false;
    }
    std::cout << "Volant connecte sur " << port << std::endl;
    return true;
}

// ── Appelé dans gameLoop() ───────────────────────────────────────────────────
void ArduinoManager::update()
{
    char char_buffer[1024];

    // ── BASE ─────────────────────────────────────────
    if (basePort && basePort->isConnected()) {
        int size = basePort->readSerialPort(char_buffer, sizeof(char_buffer));
        if (size > 0) baseBuffer.append(char_buffer, size);

        size_t pos;
        while ((pos = baseBuffer.find('\n')) != std::string::npos) {
            std::string line = baseBuffer.substr(0, pos);  // prend UNE ligne
            baseBuffer = baseBuffer.substr(pos + 1);       // garde le reste

            // Nettoie le \r si présent
            if (!line.empty() && line.back() == '\r') line.pop_back();

            if (!line.empty()) parseBase(line);
        }
    }

    // ── VOLANT ───────────────────────────────────────
    if (wheelPort && wheelPort->isConnected()) {
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
/*void ArduinoManager::update()
{
    std::string raw;

    // Lire la base
    if (basePort && basePort->isConnected()) {
        if (RcvFromSerial(basePort, raw) && raw.size() > 0)
            
            parseBase(raw)
;
    }
    // Lire le volant
    if (wheelPort && wheelPort->isConnected()) {
        if (RcvFromSerial(wheelPort, raw) && raw.size() > 0)
            parseWheel(raw);
    }
}*/

// ── Envoyer données du jeu vers le volant ────────────────────────────────────
void ArduinoManager::sendToWheel(float rpm, float maxRpm, int gear,
    float fuel, float tireWear,
    bool inPit, float speed, float angle)
{
    if (!wheelPort || !wheelPort->isConnected()) return;

    json j;
    j["rpm"] = rpm;
    j["rpmMax"] = maxRpm;
    j["gear"] = gear;
    j["fuel"] = fuel;
    j["tires"] = tireWear;
    j["speed"] = speed;
    j["pit"] = inPit;
    j["angle"] = angle;

    SendToSerial(wheelPort, j);
}

// ── Fonctions identiques au labo ─────────────────────────────────────────────
bool ArduinoManager::SendToSerial(SerialPort* port, json j_msg)
{
    std::string msg = j_msg.dump();
    return port->writeSerialPort(msg.c_str(), msg.length());
}

bool ArduinoManager::RcvFromSerial(SerialPort* port, std::string& msg)
{
    char char_buffer[MSG_MAX];
    msg.clear();

    int buffer_size = port->readSerialPort(char_buffer, MSG_MAX);
    std::string str_buffer;
    str_buffer.assign(char_buffer, buffer_size);
    msg.append(str_buffer);

    return true;
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
    }
    catch (...) {
        std::cerr << "Erreur parse volant: " << raw << std::endl;
    }
}