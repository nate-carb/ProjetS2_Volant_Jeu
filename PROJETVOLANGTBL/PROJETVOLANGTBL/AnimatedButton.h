#pragma once
#include <QPushButton>
#include <QTimer>
#include <QPainter>
#include <QGraphicsDropShadowEffect>

class AnimatedButton : public QPushButton
{
    Q_OBJECT

public:
    explicit AnimatedButton(const QString& text, QWidget* parent = nullptr);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onPulse();

private:
    QTimer* m_pulseTimer;
    float   m_glowAlpha = 0.0f;
    float   m_glowDir = 1.0f;
    bool    m_hovered = false;
    QGraphicsDropShadowEffect* m_shadow;
};
