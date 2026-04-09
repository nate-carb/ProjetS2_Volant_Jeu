#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

class ControlsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ControlsDialog(QWidget* parent = nullptr);
};
