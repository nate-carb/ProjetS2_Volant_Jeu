#include "MenuWindow.h"
#include <QSlider>
#include <QResizeEvent>
#include <QLineEdit>
#include <QTextEdit>
#include <QScrollArea>
#include "LeaderboardManager.h"
#include <QApplication>

// ===== STYLES =====
static const QString BTN_STYLE = R"(
    QPushButton {
        background-color: #111111;
        color: white;
        font-size: 22px;
        font-weight: bold;
        border-radius: 12px;
        padding: 14px;
        border: 2px solid #333333;
    }
    QPushButton:hover {
        background-color: #CC0000;
        border: 2px solid #FF3333;
    }
    QPushButton:pressed {
        background-color: #880000;
    }
)";

static const QString TITLE_STYLE = R"(
    QLabel {
        background-color: #111111;
        color: white;
        font-size: 32px;
        font-weight: bold;
        border-radius: 8px;
        padding: 14px 30px;
    }
)";

// ===== CONSTRUCTEUR =====
MenuWindow::MenuWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("F1 Racing");
    showFullScreen();

    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    m_stack->addWidget(createMainMenuPage());
    m_stack->addWidget(createTrackSelectionPage());
    m_stack->addWidget(createLeaderboardSelectionPage());
    m_stack->addWidget(createLeaderboardPage(0));
    m_stack->addWidget(createLeaderboardPage(1));
    m_stack->addWidget(createLeaderboardPage(2));

    m_stack->setCurrentIndex(PAGE_MAIN);
}

MenuWindow::~MenuWindow() {}

// ===== HELPERS =====
QPushButton* MenuWindow::createStyledButton(const QString& text)
{
    QPushButton* btn = new QPushButton(text);
    btn->setStyleSheet(BTN_STYLE);
    btn->setFixedHeight(60);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

QLabel* MenuWindow::createTitle(const QString& text)
{
    QLabel* lbl = new QLabel(text);
    lbl->setStyleSheet(TITLE_STYLE);
    lbl->setAlignment(Qt::AlignCenter);
    return lbl;
}

void MenuWindow::applyBackground(QWidget* page)
{
    page->setStyleSheet("background-color: #CC2200;");
}

void MenuWindow::goToPage(int index)
{
    m_stack->setCurrentIndex(index);
}

// ===== PAGE MAIN MENU =====
QWidget* MenuWindow::createMainMenuPage()
{
    QWidget* page = new QWidget();
    applyBackground(page);

    // GIF background
    QLabel* bgLabel = new QLabel(page);
    bgLabel->setGeometry(0, 0, 9999, 9999);
    bgLabel->setScaledContents(true);

    QMovie* movie = new QMovie("images/menu_bg.gif");
    if (movie->isValid()) {
        bgLabel->setMovie(movie);
        movie->start();
    }
    else {
        delete movie;
        bgLabel->setStyleSheet("background-color: #CC2200;");
    }
    bgLabel->lower();  // derrière tout

    // Layout par-dessus le GIF
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(15);
    layout->setContentsMargins(0, 40, 0, 40);

    // Logo F1
    QLabel* logo = new QLabel();
    QPixmap logoPx("images/f1_logo.png");
    if (!logoPx.isNull()) {
        logo->setPixmap(logoPx.scaled(380, 110,
            Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else {
        logo->setText("F1 RACING");
        logo->setStyleSheet(
            "color: white; font-size: 40px; font-weight: bold;"
            "background-color: #111111; border-radius: 8px; padding: 15px;");
    }
    logo->setAlignment(Qt::AlignCenter);
    layout->addWidget(logo);
    layout->addSpacing(20);

    QPushButton* btnPlay = createStyledButton("Play !");
    QPushButton* btnLB = createStyledButton("Leaderboards");
    QPushButton* btnOpts = createStyledButton("Options");
    QPushButton* btnCtrl = createStyledButton("Controls");
    QPushButton* btnQuit = createStyledButton("Quit");

    btnPlay->setFixedWidth(400);
    btnLB->setFixedWidth(400);
    btnOpts->setFixedWidth(400);
    btnCtrl->setFixedWidth(400);
    btnQuit->setFixedWidth(400);

    connect(btnPlay, &QPushButton::clicked, this, &MenuWindow::onPlay);
    connect(btnLB, &QPushButton::clicked, this, &MenuWindow::onLeaderboards);
    connect(btnOpts, &QPushButton::clicked, this, &MenuWindow::onOptions);
    connect(btnCtrl, &QPushButton::clicked, this, &MenuWindow::onControls);
    connect(btnQuit, &QPushButton::clicked, []() {
        QApplication::quit();
        });

    layout->addWidget(btnPlay);
    layout->addWidget(btnLB);
    layout->addWidget(btnOpts);
    layout->addWidget(btnCtrl);
    layout->addWidget(btnQuit);

    return page;
}

// ===== PAGE TRACK SELECTION =====
QWidget* MenuWindow::createTrackSelectionPage()
{
    QWidget* page = new QWidget();
    applyBackground(page);

    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(20);
    layout->setContentsMargins(40, 30, 40, 30);

    layout->addWidget(createTitle("Track Selection"));
    layout->addSpacing(10);

    QHBoxLayout* tracksLayout = new QHBoxLayout();
    tracksLayout->setSpacing(20);

    QStringList names = { "Track 1", "Track 2", "Track 3" };
    QStringList images = {
        "images/track1_preview.png",
        "images/track2_preview.png",
        "images/track3_preview.png"
    };

    for (int i = 0; i < 3; i++) {
        QVBoxLayout* box = new QVBoxLayout();
        box->setAlignment(Qt::AlignHCenter);

        QLabel* preview = new QLabel();
        QPixmap px(images[i]);
        if (px.isNull()) {
            px = QPixmap(200, 200);
            px.fill(QColor(30, 30, 30));
        }
        preview->setPixmap(px.scaled(200, 200,
            Qt::KeepAspectRatio, Qt::SmoothTransformation));
        preview->setAlignment(Qt::AlignCenter);
        preview->setStyleSheet(
            "background-color: #111111; border-radius: 8px; padding: 8px;");
        preview->setFixedSize(220, 220);

        QPushButton* btn = createStyledButton(names[i]);
        btn->setFixedWidth(220);
        int idx = i;
        connect(btn, &QPushButton::clicked, [this, idx]() {
            onTrackSelected(idx);
            });

        box->addWidget(preview);
        box->addWidget(btn);
        tracksLayout->addLayout(box);
    }

    layout->addLayout(tracksLayout);
    layout->addStretch();

    QPushButton* btnBack = createStyledButton("← Back");
    btnBack->setFixedWidth(150);
    connect(btnBack, &QPushButton::clicked, [this]() {
        goToPage(PAGE_MAIN);
        });
    layout->addWidget(btnBack, 0, Qt::AlignLeft);

    return page;
}

// ===== PAGE LEADERBOARD SELECTION =====
QWidget* MenuWindow::createLeaderboardSelectionPage()
{
    QWidget* page = new QWidget();
    applyBackground(page);

    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(15);
    layout->setContentsMargins(150, 40, 150, 40);

    layout->addWidget(createTitle("Leaderboard Selection"));
    layout->addSpacing(20);

    QStringList tracks = { "Track 1", "Track 2", "Track 3" };
    for (int i = 0; i < 3; i++) {
        QPushButton* btn = createStyledButton(tracks[i]);
        int idx = i;
        connect(btn, &QPushButton::clicked, [this, idx]() {
            onLeaderboardTrackSelected(idx);
            });
        layout->addWidget(btn);
    }

    layout->addStretch();
    QPushButton* btnBack = createStyledButton("← Back");
    btnBack->setFixedWidth(150);
    connect(btnBack, &QPushButton::clicked, [this]() {
        goToPage(PAGE_MAIN);
        });
    layout->addWidget(btnBack, 0, Qt::AlignLeft);

    return page;
}

// ===== PAGE LEADERBOARD =====
QWidget* MenuWindow::createLeaderboardPage(int trackIndex)
{
    QWidget* page = new QWidget();
    applyBackground(page);

    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(10);
    layout->setContentsMargins(40, 30, 40, 30);

    layout->addWidget(createTitle(
        QString("Track %1 Leaderboard").arg(trackIndex + 1)));
    layout->addSpacing(10);

    // Scroll area pour les scores
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: #111111; width: 8px; }"
        "QScrollBar::handle:vertical { background: #CC0000; border-radius: 4px; }");

    QWidget* content = new QWidget();
    content->setStyleSheet("background: transparent;");
    QVBoxLayout* scoreLayout = new QVBoxLayout(content);
    scoreLayout->setSpacing(8);

    QVector<LeaderboardManager::Entry> entries =
        LeaderboardManager::load(trackIndex);

    if (entries.isEmpty()) {
        QLabel* empty = new QLabel("No scores yet!");
        empty->setStyleSheet(
            "color: white; font-size: 18px; font-weight: bold;");
        empty->setAlignment(Qt::AlignCenter);
        scoreLayout->addWidget(empty);
    }
    else {
        for (int i = 0; i < entries.size(); i++) {
            QWidget* row = new QWidget();
            row->setStyleSheet(
                "background-color: #111111; border-radius: 6px;"
                "border-left: 4px solid #CC0000;");
            row->setFixedHeight(48);

            QHBoxLayout* rowLayout = new QHBoxLayout(row);
            rowLayout->setContentsMargins(15, 4, 15, 4);

            auto makeLabel = [](const QString& text, int size = 16) {
                QLabel* l = new QLabel(text);
                l->setStyleSheet(
                    QString("color: white; font-size: %1px; font-weight: bold;"
                        "background: transparent; border: none;").arg(size));
                return l;
                };

            rowLayout->addWidget(makeLabel(
                QString::number(i + 1) + ".", 18));
            rowLayout->addWidget(makeLabel(entries[i].name), 1);
            rowLayout->addStretch();
            rowLayout->addWidget(makeLabel(entries[i].formattedTime()));
            scoreLayout->addWidget(row);
        }
    }

    scoreLayout->addStretch();
    scroll->setWidget(content);
    layout->addWidget(scroll);

    QPushButton* btnBack = createStyledButton("← Back");
    btnBack->setFixedWidth(150);
    connect(btnBack, &QPushButton::clicked, [this]() {
        goToPage(PAGE_LB_SEL);
        });
    layout->addWidget(btnBack, 0, Qt::AlignLeft);

    return page;
}

// ===== SLOTS =====
void MenuWindow::onPlay() { goToPage(PAGE_TRACK_SEL); }
void MenuWindow::onLeaderboards() { goToPage(PAGE_LB_SEL); }

void MenuWindow::onOptions() {
    OptionsDialog dlg(this);
    dlg.exec();  // bloquant — retourne au menu après fermeture
}

void MenuWindow::onControls() {
    ControlsDialog dlg(this);
    dlg.exec();
}

void MenuWindow::onTrackSelected(int trackIndex) {
    emit playRequested(trackIndex);
}

void MenuWindow::onLeaderboardTrackSelected(int trackIndex)
{
    // Remplace la page existante par une nouvelle à jour
    QWidget* oldPage = m_stack->widget(PAGE_LB_T1 + trackIndex);
    m_stack->removeWidget(oldPage);
    delete oldPage;

    QWidget* newPage = createLeaderboardPage(trackIndex);
    m_stack->insertWidget(PAGE_LB_T1 + trackIndex, newPage);

    goToPage(PAGE_LB_T1 + trackIndex);
}