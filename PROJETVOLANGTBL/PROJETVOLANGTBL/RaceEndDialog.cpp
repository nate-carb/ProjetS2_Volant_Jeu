#include "RaceEndDialog.h"
#include "AnimatedButton.h"

RaceEndDialog::RaceEndDialog(int trackIndex, int playerTimeMs, QWidget* parent)
    : QDialog(parent), m_trackIndex(trackIndex), m_playerTimeMs(playerTimeMs)
{
    setWindowTitle("Course terminée!");
    setFixedSize(500, 650);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");

    // Widget central avec coins ronds
    QWidget* card = new QWidget(this);
    card->setStyleSheet(
        "QWidget { background-color: #1a0030;"
        "border-radius: 20px; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(card);

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setSpacing(12);
    layout->setContentsMargins(30, 20, 30, 20);

    // ===== TITRE =====
    QStringList trackNames = { "Classement Espace", "Classement Monaco", "Classement Plage" };
    QLabel* title = new QLabel(trackNames[trackIndex]);
    title->setStyleSheet(
        "background-color: #111111; color: white; font-size: 22px;"
        "font-weight: bold; border-radius: 8px; padding: 12px;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // ===== TEMPS DU JOUEUR =====
    LeaderboardManager::Entry playerEntry;
    playerEntry.timeMs = playerTimeMs;

    QLabel* yourTime = new QLabel(
        QString("Votre temps: %1").arg(playerEntry.formattedTime()));
    yourTime->setStyleSheet(
        "color: white; font-size: 18px; font-weight: bold;"
        "background-color: #4B0082; border-radius: 6px; padding: 8px;");
    yourTime->setAlignment(Qt::AlignCenter);
    layout->addWidget(yourTime);

    // ===== LEADERBOARD SCROLLABLE =====
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: #111111; width: 8px; }"
        "QScrollBar::handle:vertical { background: #4B0082; border-radius: 4px; }");

    QWidget* scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;");
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(6);
    scrollLayout->setContentsMargins(0, 0, 0, 0);

    buildLeaderboard(scrollLayout);

    scroll->setWidget(scrollContent);
    layout->addWidget(scroll);

    // ===== ENTRER SON NOM =====
    QLabel* nameLabel = new QLabel("Entrez votre nom:");
    nameLabel->setStyleSheet(
        "color: white; font-size: 14px; font-weight: bold;"
        "background: transparent;");
    layout->addWidget(nameLabel);

    m_nameInput = new QLineEdit();
    m_nameInput->setMaxLength(20);
    m_nameInput->setPlaceholderText("Votre nom...");
    m_nameInput->setStyleSheet(
        "background-color: #111111; color: white; font-size: 16px;"
        "border-radius: 8px; padding: 8px; border: 2px solid #4B0082;");
    layout->addWidget(m_nameInput);

    // ===== BOUTONS =====
    QPushButton* btnSubmit = createStyledButton("Sauvegarder");
    connect(btnSubmit, &QPushButton::clicked, [this, btnSubmit]() {
        QString name = m_nameInput->text().trimmed();
        if (name.isEmpty()) name = "???";
        LeaderboardManager::addEntry(m_trackIndex, name, m_playerTimeMs);
        btnSubmit->setText("Sauvegardé ✓");
        btnSubmit->setEnabled(false);
        });
    layout->addWidget(btnSubmit);

    QPushButton* btnClose = createStyledButton("Fermer");
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(btnClose);
}

void RaceEndDialog::buildLeaderboard(QVBoxLayout* layout)
{
    QVector<LeaderboardManager::Entry> entries =
        LeaderboardManager::load(m_trackIndex);

    if (entries.isEmpty()) {
        QLabel* empty = new QLabel("Aucun score pour l'instant!");
        empty->setStyleSheet(
            "color: white; font-size: 14px; background: transparent;");
        empty->setAlignment(Qt::AlignCenter);
        layout->addWidget(empty);
        return;
    }

    for (int i = 0; i < entries.size(); i++) {
        QWidget* row = new QWidget();
        row->setStyleSheet(
            "background-color: #111111; border-radius: 6px;"
            "border-left: 4px solid #4B0082;");
        row->setFixedHeight(40);

        QHBoxLayout* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(12, 4, 12, 4);

        auto makeLabel = [](const QString& text, int size = 14) {
            QLabel* l = new QLabel(text);
            l->setStyleSheet(
                QString("color: white; font-size: %1px; font-weight: bold;"
                    "background: transparent; border: none;").arg(size));
            return l;
            };

        if (entries[i].timeMs == m_playerTimeMs)
            row->setStyleSheet(
                "background-color: #2d0060; border-radius: 6px;"
                "border-left: 4px solid #8B00FF;");

        rowLayout->addWidget(makeLabel(QString::number(i + 1) + ".", 16));
        rowLayout->addWidget(makeLabel(entries[i].name));
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
    accept();
}

QPushButton* RaceEndDialog::createStyledButton(const QString& text)
{
    AnimatedButton* btn = new AnimatedButton(text);
    btn->setFixedHeight(50);
    return btn;
}