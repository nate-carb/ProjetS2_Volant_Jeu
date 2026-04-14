#include "AnimatedButton.h"
#include <QPainterPath>

AnimatedButton::AnimatedButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setContentsMargins(10, 5, 10, 5);  
    m_shadow = new QGraphicsDropShadowEffect(this);
    m_shadow->setBlurRadius(0);
    m_shadow->setColor(QColor(0, 200, 255, 0));
    m_shadow->setOffset(0, 0);
    setGraphicsEffect(m_shadow);
    setStyleSheet(R"(
    QPushButton {
        background-color: #111111;
        color: white;
        font-weight: bold;
        border-radius: 12px;
        padding: 14px;
        border: 2px solid #333333;
    }
    QPushButton:hover {
        background-color: #4B0082;
        border: 2px solid #8B00FF;
    }
    QPushButton:pressed {
        background-color: #2D0052;
    }
)");

    setCursor(Qt::PointingHandCursor);

    m_pulseTimer = new QTimer(this);
    m_pulseTimer->setInterval(16);  // ~60fps
    connect(m_pulseTimer, &QTimer::timeout, this, &AnimatedButton::onPulse);
}

void AnimatedButton::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    m_pulseTimer->start();
    QPushButton::enterEvent(event);
}

void AnimatedButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    QPushButton::leaveEvent(event);
}

void AnimatedButton::onPulse()
{
    if (m_hovered) {
        // Pulse entre 0.3 et 1.0
        m_glowAlpha += m_glowDir * 0.05f;
        if (m_glowAlpha >= 1.0f) {
            m_glowAlpha = 1.0f;
            m_glowDir = -1.0f;
        }
        else if (m_glowAlpha <= 0.3f) {
            m_glowAlpha = 0.3f;
            m_glowDir = 1.0f;
        }
    }
    else {
        // Fade out
        m_glowAlpha -= 0.08f;
        if (m_glowAlpha <= 0.0f) {
            m_glowAlpha = 0.0f;
            m_pulseTimer->stop();
        }
    }
    m_shadow->setBlurRadius(m_glowAlpha * 120.0f);
    m_shadow->setColor(QColor(0, 200, 255, int(m_glowAlpha * 255)));
}

void AnimatedButton::paintEvent(QPaintEvent* event)
{
    QPushButton::paintEvent(event);
}