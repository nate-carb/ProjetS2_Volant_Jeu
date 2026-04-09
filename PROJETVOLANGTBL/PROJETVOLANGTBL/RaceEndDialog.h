#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include "LeaderboardManager.h"

class RaceEndDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RaceEndDialog(int trackIndex, int playerTimeMs,
        QWidget* parent = nullptr);

private slots:
    void onSubmit();

private:
    int m_trackIndex;
    int m_playerTimeMs;
    QLineEdit* m_nameInput;

    void buildLeaderboard(QVBoxLayout* layout);
    QPushButton* createStyledButton(const QString& text);
};
