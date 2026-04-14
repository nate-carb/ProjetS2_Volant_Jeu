#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include "SoundManager.h"

class OptionsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit OptionsDialog(SoundManager* soundManager = nullptr, QWidget* parent = nullptr);
    int getVolume() const { return m_volume; }

signals:
    void volumeChanged(int volume);

private:
    int m_volume = 80;
    SoundManager* m_soundManager = nullptr;
};
