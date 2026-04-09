#include "RaceEndDialog.h"

static const QString BTN_STYLE = R"(
    QPushButton {
        background-color: #111111;
        color: white;
        font-size: 20px;
        font-weight: bold;
        border-radius: 10px;
        padding: 12px;
        border: 2px solid #333;
    }
    QPushButton:hover {
        background-color: #CC0000;
        border: 2px solid #FF3333;
    }
)";

RaceEndDialog::RaceEndDialog(int trackIndex, int playerTimeMs, QWidget* parent)
    : QDialog(parent), m_trackIndex(trackIndex), m_playerTimeMs(playerTimeMs)
{
    setWindowTitle("Race Finished!");
    setFixedSize(500, 650);
    setStyleSheet("background-color: #CC2200;");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(12);
    layout->setContentsMargins(30, 20, 30, 20);

    // ===== TITRE =====
    QLabel* title = new QLabel(QString("Track %1 Leaderboard")
        .arg(trackIndex + 1));
    title->setStyleSheet(
        "background-color: #111111; color: white; font-size: 26px;"
        "font-weight: bold; border-radius: 8px; padding: 12px;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // ===== TEMPS DU JOUEUR =====
    LeaderboardManager::Entry playerEntry;
    playerEntry.timeMs = playerTimeMs;

    QLabel* yourTime = new QLabel(
        QString("Your time: %1").arg(playerEntry.formattedTime()));
    yourTime->setStyleSheet(
        "color: white; font-size: 20px; font-weight: bold;"
        "background-color: #880000; border-radius: 6px; padding: 8px;");
    yourTime->setAlignment(Qt::AlignCenter);
    layout->addWidget(yourTime);

    // ===== LEADERBOARD SCROLLABLE =====
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: #111111; width: 8px; }"
        "QScrollBar::handle:vertical { background: #CC0000; border-radius: 4px; }");

    QWidget* scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;");
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(6);
    scrollLayout->setContentsMargins(0, 0, 0, 0);

    buildLeaderboard(scrollLayout);

    scroll->setWidget(scrollContent);
    layout->addWidget(scroll);

    // ===== ENTRER SON NOM =====
    QLabel* nameLabel = new QLabel("Enter your name:");
    nameLabel->setStyleSheet(
        "color: white; font-size: 16px; font-weight: bold;");
    layout->addWidget(nameLabel);

    m_nameInput = new QLineEdit();
    m_nameInput->setMaxLength(20);
    m_nameInput->setPlaceholderText("Your name...");
    m_nameInput->setStyleSheet(
        "background-color: #111111; color: white; font-size: 18px;"
        "border-radius: 8px; padding: 8px; border: 2px solid #333;");
    layout->addWidget(m_nameInput);

    // ===== BOUTONS =====
    QPushButton* btnSubmit = createStyledButton("Save Score");
    connect(btnSubmit, &QPushButton::clicked, [this, btnSubmit]() {
        QString name = m_nameInput->text().trimmed();
        if (name.isEmpty()) name = "???";
        LeaderboardManager::addEntry(m_trackIndex, name, m_playerTimeMs);
        btnSubmit->setText("Saved ✓");
        btnSubmit->setEnabled(false);
        });
    layout->addWidget(btnSubmit);

    QPushButton* btnClose = createStyledButton("Close");
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(btnClose);
}
void RaceEndDialog::buildLeaderboard(QVBoxLayout* layout)
{
    QVector<LeaderboardManager::Entry> entries =
        LeaderboardManager::load(m_trackIndex);

    if (entries.isEmpty()) {
        QLabel* empty = new QLabel("No scores yet!");
        empty->setStyleSheet("color: white; font-size: 16px;");
        empty->setAlignment(Qt::AlignCenter);
        layout->addWidget(empty);
        return;
    }

    for (int i = 0; i < entries.size(); i++) {
        QWidget* row = new QWidget();
        row->setStyleSheet(
            "background-color: #111111; border-radius: 6px;"
            "border-left: 4px solid #CC0000;");
        row->setFixedHeight(45);

        QHBoxLayout* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(12, 4, 12, 4);

        auto makeLabel = [](const QString& text, int size = 16) {
            QLabel* l = new QLabel(text);
            l->setStyleSheet(
                QString("color: white; font-size: %1px; font-weight: bold;"
                    "background: transparent; border: none;").arg(size));
            return l;
            };

        // Highlight le joueur actuel si son temps match
        if (entries[i].timeMs == m_playerTimeMs)
            row->setStyleSheet(
                "background-color: #440000; border-radius: 6px;"
                "border-left: 4px solid #FF0000;");

        rowLayout->addWidget(makeLabel(QString::number(i + 1) + ".", 18));
        rowLayout->addWidget(makeLabel(entries[i].name), 1);
        rowLayout->addStretch();
        rowLayout->addWidget(makeLabel(entries[i].formattedTime()));

        layout->addWidget(row);
    }
}

void RaceEndDialog::onSubmit()
{
    QString name = m_nameInput->text().trimmed();
    if (name.isEmpty()) name = "???";

    LeaderboardManager::addEntry(m_trackIndex, name, m_playerTimeMs);

    // Refresh le leaderboard affiché
    accept();
}

QPushButton* RaceEndDialog::createStyledButton(const QString& text)
{
    QPushButton* btn = new QPushButton(text);
    btn->setStyleSheet(BTN_STYLE);
    btn->setFixedHeight(50);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}