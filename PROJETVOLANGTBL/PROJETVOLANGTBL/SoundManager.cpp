#include "SoundManager.h"
#include <algorithm>
#include <cmath>

SoundManager::SoundManager(QObject* parent) : QObject(parent)
{
    menuMusic = new QMediaPlayer(this);
    menuOutput = new QAudioOutput(this);
    menuMusic->setAudioOutput(menuOutput);
    menuMusic->setSource(QUrl::fromLocalFile("sounds/musique_menu.wav"));
    menuMusic->setLoops(QMediaPlayer::Infinite);
    menuOutput->setVolume(0.5f);
    // ── Moteur (boucle) ──────────────────────────────────────
    enginePlayer = new QMediaPlayer(this);
    engineOutput = new QAudioOutput(this);
    enginePlayer->setAudioOutput(engineOutput);
    //enginePlayer->setSource(QUrl::fromLocalFile("sounds/engine_loop.wav"));
    enginePlayer->setLoops(QMediaPlayer::Infinite);
    engineOutput->setVolume(0.7f);
    enginePlayer->play();

    // ── Freinage ─────────────────────────────────────────────
    brakeSound = new QSoundEffect(this);
    brakeSound->setSource(QUrl::fromLocalFile("sounds/brake.wav"));
    brakeSound->setVolume(0.6f);

    // ── Changement de vitesse ─────────────────────────────────
    shiftSound = new QSoundEffect(this);
    shiftSound->setSource(QUrl::fromLocalFile("sounds/gear_shift.wav"));
    shiftSound->setVolume(0.5f);

    // ── NOS ───────────────────────────────────────────────────
    nosSound = new QSoundEffect(this);
    nosSound->setSource(QUrl::fromLocalFile("sounds/nos.wav"));
    nosSound->setVolume(0.8f);

    // ── Herbe ────────────────────────────────────────────────
    grassSound = new QSoundEffect(this);
    grassSound->setSource(QUrl::fromLocalFile("sounds/grass.wav"));
    grassSound->setVolume(0.4f);
    grassSound->setLoopCount(QSoundEffect::Infinite);
}

void SoundManager::updateEngine(float rpm, float maxRpm, bool isOnGrass)
{
    float ratio = std::clamp(rpm / maxRpm, 0.0f, 1.0f);

    float targetPitch = 0.5f + ratio * 2.0f;
    float targetVolume = 0.3f + ratio * 0.7f;
    if (isOnGrass) targetVolume *= 0.6f;

    // Lerp pour lisser (0.05 = très doux, 0.15 = plus réactif)
    currentPitch += (targetPitch - currentPitch) * 0.05f;
    currentVolume += (targetVolume - currentVolume) * 0.05f;

    // Mise à jour seulement si le changement est visible
    if (std::abs(enginePlayer->playbackRate() - currentPitch) > 0.01f)
        enginePlayer->setPlaybackRate(currentPitch);

    if (std::abs(engineOutput->volume() - currentVolume) > 0.01f)
        engineOutput->setVolume(currentVolume);
}

void SoundManager::playBrake(bool isBraking, float speed)
{
    // Front montant : début du freinage à vitesse suffisante
    if (isBraking && !wasBraking && speed > 5.0f) {
        brakeSound->play();
    }
    wasBraking = isBraking;
}

void SoundManager::playGearShift()
{
    shiftSound->play();
}

void SoundManager::playNos(bool isBoosting)
{
    if (isBoosting && !wasBoosting) {
        nosSound->play();
    }
    wasBoosting = isBoosting;
}

void SoundManager::playGrass(bool isOnGrass)
{
    if (isOnGrass && !wasOnGrass)  grassSound->play();
    if (!isOnGrass && wasOnGrass)  grassSound->stop();
    wasOnGrass = isOnGrass;
}
void SoundManager::playMenuMusic()
{
    menuMusic->play();
}

void SoundManager::stopMenuMusic()
{
    menuMusic->stop();
}