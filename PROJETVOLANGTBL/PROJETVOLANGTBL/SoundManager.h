#pragma once
#include <QObject>
#include <QMediaPlayer>
#include <QSoundEffect>
#include <QUrl>
#include <QAudioOutput>

class SoundManager : public QObject {
    Q_OBJECT
public:
    explicit SoundManager(QObject* parent = nullptr);

    void updateEngine(float rpm, float maxRpm, bool isOnGrass);
    void playBrake(bool isBraking, float speed);
    void playGearShift();
    void playNos(bool isBoosting);
    void playGrass(bool isOnGrass);
    QMediaPlayer* menuMusic = nullptr;
    QAudioOutput* menuOutput = nullptr;
    void playMenuMusic();
    void stopMenuMusic();

private:
    // Moteur (son en boucle dont on change la vitesse selon RPM)
    QMediaPlayer* enginePlayer = nullptr;
    QAudioOutput* engineOutput = nullptr;

    // Sons one-shot
    QSoundEffect* brakeSound = nullptr;
    QSoundEffect* shiftSound = nullptr;
    QSoundEffect* nosSound = nullptr;
    QSoundEffect* grassSound = nullptr;

    bool  wasBraking = false;
    bool  wasBoosting = false;
    bool  wasOnGrass = false;
    float currentPitch = 1.0f;
    float currentVolume = 0.5f;
};
