#include "MenuWindow.h"
#include <QSlider>
#include <QResizeEvent>
#include <QLineEdit>
#include <QTextEdit>
#include <QScrollArea>
#include "LeaderboardManager.h"
#include <QApplication>
#include <QStackedLayout>
#include <QScreen>
#include <QGuiApplication>
#include <QFontDatabase>

#include "AnimatedButton.h"

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

MenuWindow::MenuWindow(SoundManager* soundManager, ArduinoManager* arduino, QWidget* parent)
    : QMainWindow(parent), m_soundManager(soundManager)
{
    int fontId = QFontDatabase::addApplicationFont("font/PressStart2P-Regular.ttf");
    QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont gameFont(fontFamily, 10);
    QApplication::setFont(gameFont);  

    setWindowTitle("F1 Racing");
    showFullScreen();
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
    m_stack->setCurrentIndex(0);

    m_arduino = arduino;
}

MenuWindow::~MenuWindow() {}

QPushButton* MenuWindow::createStyledButton(const QString& text)
{
    AnimatedButton* btn = new AnimatedButton(text);
    btn->setFixedHeight(60);
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
    // Focus sur le premier bouton de la page
    QWidget* page = m_stack->currentWidget();
    QList<QPushButton*> buttons = page->findChildren<QPushButton*>();
    if (!buttons.isEmpty())
        buttons.first()->setFocus();
}

QWidget* MenuWindow::createMainMenuPage()
{
    QWidget* page = new QWidget();
    applyBackground(page);

    QScreen* screen = QGuiApplication::primaryScreen();
    QSize screenSize = screen->size();

    QLabel* bgLabel = new QLabel(page);
    bgLabel->setScaledContents(true);
    bgLabel->setGeometry(0, 0, screenSize.width(), screenSize.height());
    QMovie* movie = new QMovie("images/menu/backgroundmenu1.gif");
    if (movie->isValid()) {
        bgLabel->setMovie(movie);
        movie->start();
    }
    else {
        delete movie;
    }
    bgLabel->lower();

    // Layout principal
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    // Widget centré qui contient tout
    QWidget* centerWidget = new QWidget();
    centerWidget->setStyleSheet("background: transparent;");
    centerWidget->setFixedWidth(480);  
    QVBoxLayout* centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->setAlignment(Qt::AlignCenter);
    centerLayout->setSpacing(15);
    centerLayout->setContentsMargins(20, 40, 20, 40);

    layout->addWidget(centerWidget, 0, Qt::AlignHCenter);

    // Logo GIF
    QLabel* logo = new QLabel();
    logo->setFixedSize(384, 195);
    logo->setScaledContents(true);
    logo->setStyleSheet("background: transparent;");
    logo->setAlignment(Qt::AlignCenter);
    QMovie* logoMovie = new QMovie("images/menu/f1.gif");
    if (logoMovie->isValid()) {
        logo->setMovie(logoMovie);
        logoMovie->start();
    }
    else {
        delete logoMovie;
        logo->setText("F1 RACING");
        logo->setStyleSheet(
            "color: white; font-size: 40px; font-weight: bold;"
            "background-color: #111111; border-radius: 8px; padding: 15px;");
    }
    centerLayout->addWidget(logo, 0, Qt::AlignHCenter);
    centerLayout->addSpacing(5);

    // Boutons
    QPushButton* btnPlay = createStyledButton("Jouer !");
    QPushButton* btnLB = createStyledButton("Classements");
    QPushButton* btnOpts = createStyledButton("Options");
    QPushButton* btnCtrl = createStyledButton("Contrôles");
    QPushButton* btnQuit = createStyledButton("Quitter");

    btnPlay->setFixedWidth(400);
    btnLB->setFixedWidth(400);
    btnOpts->setFixedWidth(400);
    btnCtrl->setFixedWidth(400);
    btnQuit->setFixedWidth(400);

    connect(btnPlay, &QPushButton::clicked, this, &MenuWindow::onPlay);
    connect(btnLB, &QPushButton::clicked, this, &MenuWindow::onLeaderboards);
    connect(btnOpts, &QPushButton::clicked, this, &MenuWindow::onOptions);
    connect(btnCtrl, &QPushButton::clicked, this, &MenuWindow::onControls);
    connect(btnQuit, &QPushButton::clicked, []() { QApplication::quit(); });

    centerLayout->addWidget(btnPlay);
    centerLayout->addWidget(btnLB);
    centerLayout->addWidget(btnOpts);
    centerLayout->addWidget(btnCtrl);
    centerLayout->addWidget(btnQuit);

    btnPlay->setFocus();

    return page;
}

QWidget* MenuWindow::createTrackSelectionPage()
{
    QWidget* page = new QWidget();
    QScreen* screen = QGuiApplication::primaryScreen();
    QSize screenSize = screen->size();
    QLabel* bgLabel = new QLabel(page);
    bgLabel->setScaledContents(true);
    bgLabel->setGeometry(0, 0, screenSize.width(), screenSize.height());
    QMovie* movie = new QMovie("images/tracks/background2.gif");
    if (movie->isValid()) {
        bgLabel->setMovie(movie);
        movie->start();
    }
    else {
        delete movie;
    }
    bgLabel->lower();
    applyBackground(page);

    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(20);
    layout->setContentsMargins(150, 30, 150, 30);

    QLabel* titleLabel = new QLabel("Sélection de piste");
    titleLabel->setStyleSheet(
        "background-color: #111111; color: white; font-size: 24px;"
        "font-weight: bold; border-radius: 8px; padding: 10px 20px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedWidth(600);
    layout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(10);

    QHBoxLayout* tracksLayout = new QHBoxLayout();
    tracksLayout->setSpacing(20);

    QStringList names = { "Espace", "Monaco", "Plage" };
    QStringList imagePaths = {
        "images/tracks/track1.png",
        "images/tracks/track2.png",
        "images/tracks/track3.png"
    };

    for (int i = 0; i < 3; i++) {
        QVBoxLayout* box = new QVBoxLayout();
        box->setAlignment(Qt::AlignHCenter);

        QLabel* preview = new QLabel();
        QPixmap px(imagePaths[i]);
        if (px.isNull()) {
            px = QPixmap(200, 200);
            px.fill(QColor(30, 30, 30));
        }
        preview->setPixmap(px.scaled(370, 280,
            Qt::KeepAspectRatio, Qt::SmoothTransformation));
        preview->setAlignment(Qt::AlignCenter);
        preview->setStyleSheet("background: transparent;");
        preview->setFixedSize(380, 290);

        QPushButton* btn = createStyledButton(names[i]);
        btn->setFixedWidth(380);
        int idx = i;
        connect(btn, &QPushButton::clicked, [this, idx]() {
            onTrackSelected(idx);
            });

        box->addWidget(preview);
        box->addWidget(btn);
        tracksLayout->addLayout(box);
    }

    layout->addStretch();
    layout->addLayout(tracksLayout);
    layout->addStretch();

    QPushButton* btnBack = createStyledButton("← Retour");
    btnBack->setFixedWidth(220);
    connect(btnBack, &QPushButton::clicked, [this]() {
        goToPage(PAGE_MAIN);
        });
    layout->addWidget(btnBack, 0, Qt::AlignLeft);

    return page;
}

QWidget* MenuWindow::createLeaderboardSelectionPage()
{
    QWidget* page = new QWidget();
    applyBackground(page);

    QScreen* screen = QGuiApplication::primaryScreen();
    QSize screenSize = screen->size();
    QLabel* bgLabel = new QLabel(page);
    bgLabel->setScaledContents(true);
    bgLabel->setGeometry(0, 0, screenSize.width(), screenSize.height());
    QMovie* movie = new QMovie("images/classements/background1.gif");
    if (movie->isValid()) {
        bgLabel->setMovie(movie);
        movie->start();
    }
    else {
        delete movie;
    }
    bgLabel->lower();

    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(20);
    layout->setContentsMargins(150, 30, 150, 30);

    // Titre
    QLabel* titleLabel = new QLabel("Sélection de classement");
    titleLabel->setStyleSheet(
        "background-color: #111111; color: white; font-size: 24px;"
        "font-weight: bold; border-radius: 8px; padding: 10px 20px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedWidth(600);
    layout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(10);

    // 3 tracks côte à côte
    QHBoxLayout* tracksLayout = new QHBoxLayout();
    tracksLayout->setSpacing(20);

    QStringList names = { "Classement Espace", "Classement Monaco", "Classement Plage" };
    QStringList imagePaths = {
        "images/tracks/track1.png",
        "images/tracks/track2.png",
        "images/tracks/track3.png"
    };

    for (int i = 0; i < 3; i++) {
        QVBoxLayout* box = new QVBoxLayout();
        box->setSpacing(10);
        box->setAlignment(Qt::AlignHCenter);

        QLabel* preview = new QLabel();
        QPixmap px(imagePaths[i]);
        if (px.isNull()) {
            px = QPixmap(380, 290);
            px.fill(QColor(30, 30, 30));
        }
        preview->setPixmap(px.scaled(370, 280,
            Qt::KeepAspectRatio, Qt::SmoothTransformation));
        preview->setAlignment(Qt::AlignCenter);
        preview->setStyleSheet("background: transparent;");
        preview->setFixedSize(380, 290);

        QPushButton* btn = createStyledButton(names[i]);
        btn->setFixedWidth(380);
        int idx = i;
        connect(btn, &QPushButton::clicked, [this, idx]() {
            onLeaderboardTrackSelected(idx);
            });

        box->addWidget(preview);
        box->addWidget(btn);
        tracksLayout->addLayout(box);
    }

    layout->addStretch();
    layout->addLayout(tracksLayout);
    layout->addStretch();

    QPushButton* btnBack = createStyledButton("← Retour");
    btnBack->setFixedWidth(220);
    connect(btnBack, &QPushButton::clicked, [this]() {
        goToPage(PAGE_MAIN);
        });
    layout->addWidget(btnBack, 0, Qt::AlignLeft);

    return page;
}

QWidget* MenuWindow::createLeaderboardPage(int trackIndex)
{
    QWidget* page = new QWidget();
    QScreen* screen = QGuiApplication::primaryScreen();
    QSize screenSize = screen->size();
    QLabel* bgLabel = new QLabel(page);
    bgLabel->setScaledContents(true);
    bgLabel->setGeometry(0, 0, screenSize.width(), screenSize.height());
    QStringList bgGifs = {
     "images/classements/spacebg.gif",
     "images/classements/monacobg.gif",
     "images/classements/beachbg.gif"
    };
    QMovie* movie = new QMovie(bgGifs[trackIndex]);
    if (movie->isValid()) {
        bgLabel->setMovie(movie);
        movie->start();
    }
    else {
        delete movie;
    }
    bgLabel->lower();

    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(5);
    layout->setContentsMargins(200, 30, 200, 30);

    QStringList trackNames = { "Classement Espace", "Classement Monaco", "Classement Plage" };
    QLabel* titleLabel = new QLabel(trackNames[trackIndex]);
    titleLabel->setStyleSheet(
        "background-color: #111111; color: white; font-size: 24px;"
        "font-weight: bold; border-radius: 8px; padding: 10px 20px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedWidth(600);
    layout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(10);

    // Scroll area pour les scores
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    QStringList scrollColors = { "#8B00FF", "#FF7893", "#08CBFC" };  // mauve, orange, bleu

    scroll->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: #111111; width: 8px; }"
        "QScrollBar::handle:vertical { background: " + scrollColors[trackIndex] + "; border-radius: 4px; }");

    QWidget* content = new QWidget();
    content->setStyleSheet("background: transparent;");
    QVBoxLayout* scoreLayout = new QVBoxLayout(content);
    scoreLayout->setSpacing(8);
    scoreLayout->setAlignment(Qt::AlignHCenter);

    QVector<LeaderboardManager::Entry> entries =
        LeaderboardManager::load(trackIndex);

    scroll->setFocusPolicy(Qt::NoFocus);
    scroll->verticalScrollBar()->setFocusPolicy(Qt::NoFocus);
    content->setFocusPolicy(Qt::NoFocus);

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
            QStringList borderColors = { "#8B00FF", "#FF7893", "#08CBFC" };
            row->setStyleSheet(
                "background-color: #111111; border-radius: 6px;"
                "border-left: 4px solid " + borderColors[trackIndex] + ";");
            row->setFixedHeight(40);
            row->setMaximumWidth(600);

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
            rowLayout->addWidget(makeLabel(entries[i].name));
            rowLayout->addStretch();
            rowLayout->addWidget(makeLabel(entries[i].formattedTime()));
            scoreLayout->addWidget(row);
        }
    }

    scoreLayout->addStretch();
    scroll->setWidget(content);
    layout->addWidget(scroll);

    QPushButton* btnBack = createStyledButton("← Retour");
    btnBack->setFixedWidth(220);
    connect(btnBack, &QPushButton::clicked, [this]() {
        goToPage(PAGE_LB_SEL);
        });
    layout->addWidget(btnBack, 0, Qt::AlignLeft);

    return page;
}

void MenuWindow::onPlay() { goToPage(PAGE_TRACK_SEL); }
void MenuWindow::onLeaderboards() { goToPage(PAGE_LB_SEL); }

void MenuWindow::onOptions() {
    OptionsDialog* dlg = new OptionsDialog(m_soundManager, this);
    dlg->show();
}

void MenuWindow::onControls() {
    ControlsDialog* dlg = new ControlsDialog(this);
    dlg->show();
}

void MenuWindow::onTrackSelected(int trackIndex) {
    emit playRequested(trackIndex);
}

void MenuWindow::onLeaderboardTrackSelected(int trackIndex)
{
    QWidget* oldPage = m_stack->widget(PAGE_LB_T1 + trackIndex);
    m_stack->removeWidget(oldPage);
    delete oldPage;

    QWidget* newPage = createLeaderboardPage(trackIndex);
    m_stack->insertWidget(PAGE_LB_T1 + trackIndex, newPage);

    goToPage(PAGE_LB_T1 + trackIndex);
}

void MenuWindow::pollEncoder()
{
    if (!m_arduino) return;

    
    ArduinoWheelData wd = m_arduino->getWheelData();

    if (!m_prevEnc2Init) {
        m_prevEnc2 = wd.enc2;
        m_prevEnc2Init = true;
        return;
    }

    int delta = wd.enc2 - m_prevEnc2;
    if (delta == 0) return;
    m_prevEnc2 = wd.enc2;

    int page = m_stack->currentIndex();
    int lbIdx = -1;
    if (page == PAGE_LB_T1) lbIdx = 0;
    else if (page == PAGE_LB_T2) lbIdx = 1;
    else if (page == PAGE_LB_T3) lbIdx = 2;
    if (lbIdx < 0) return;

    QScrollArea* scroll = m_lbScrolls[lbIdx];
    if (!scroll) return;

    QScrollBar* vbar = scroll->verticalScrollBar();
    int step = 20;  
    vbar->setValue(vbar->value() + delta * step);
}

void MenuWindow::showEvent(QShowEvent* e)
{
    QMainWindow::showEvent(e);
    if (m_encoderTimer) m_encoderTimer->start(30);
    m_prevEnc2Init = false;
}

void MenuWindow::hideEvent(QHideEvent* e)
{
    QMainWindow::hideEvent(e);
    if (m_encoderTimer) m_encoderTimer->stop();
}