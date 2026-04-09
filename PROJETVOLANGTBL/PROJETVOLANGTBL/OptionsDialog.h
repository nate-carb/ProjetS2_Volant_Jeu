#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>

class OptionsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit OptionsDialog(QWidget* parent = nullptr);
    int getVolume() const { return m_volume; }

private:
    int m_volume = 80;
};
