#include "mainwindow.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QStatusBar>
#include <QApplication>
#include <QFrame>
#include <ctime>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Quantitative Platform");
    resize(1400, 900);

    setupTheme();

    auto *central = new QWidget;
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ---- LEFT NAVBAR ----
    auto *navbar = new QFrame;
    navbar->setFixedWidth(70);
    navbar->setObjectName("navbar");
    auto *navLayout = new QVBoxLayout(navbar);
    navLayout->setContentsMargins(4, 10, 4, 10);
    navLayout->setSpacing(6);

    // App logo/icon
    auto *logo = new QLabel("QP");
    logo->setAlignment(Qt::AlignCenter);
    logo->setStyleSheet("font-size: 18px; font-weight: bold; color: #26a65b; padding: 8px;");
    navLayout->addWidget(logo);

    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #333;");
    navLayout->addWidget(sep);

    struct NavItem { QString icon; QString label; };
    const QVector<NavItem> items = {
        {"📈", " 行情"},
        {"🧠", " 策略"},
        {"⏮", " 回测"},
        {"💹", " 交易"},
        {"👥", " 账户"},
    };

    for (int i = 0; i < items.size(); ++i) {
        auto *btn = new QPushButton(items[i].icon + "\n" + items[i].label);
        btn->setCheckable(true);
        btn->setFixedHeight(60);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setObjectName("navBtn");
        if (i == 0) btn->setChecked(true);
        m_navBtns.append(btn);
        navLayout->addWidget(btn);
        connect(btn, &QPushButton::clicked, [this, i]() { switchPage(i); });
    }
    navLayout->addStretch();

    // Version label at bottom
    auto *ver = new QLabel("v0.5");
    ver->setAlignment(Qt::AlignCenter);
    ver->setStyleSheet("color: #555; font-size: 10px;");
    navLayout->addWidget(ver);

    layout->addWidget(navbar);

    // ---- Divider ----
    auto *divider = new QFrame;
    divider->setFrameShape(QFrame::VLine);
    divider->setFixedWidth(1);
    divider->setStyleSheet("background-color: #333;");
    layout->addWidget(divider);

    // ---- CONTENT STACK ----
    m_stack = new QStackedWidget;

    m_chartPanel    = new ChartPanel;
    m_strategyPanel = new StrategyPanel;
    m_backtestPanel = new BacktestPanel;

    m_tradePanel = new TradePanel;

    m_accountPanel = new AccountPanel;

    m_stack->addWidget(m_chartPanel);       // 0
    m_stack->addWidget(m_strategyPanel);    // 1
    m_stack->addWidget(m_backtestPanel);    // 2
    m_stack->addWidget(m_tradePanel);       // 3
    m_stack->addWidget(m_accountPanel);     // 4

    layout->addWidget(m_stack);
    setCentralWidget(central);

    // ---- Status bar ----
    statusBar()->showMessage("Ready  |  Data: SQLite  |  Engine: Phase 3  |  UI: Phase 4");
    statusBar()->setStyleSheet("QStatusBar { background-color: #1a1a1a; color: #888; }");

    // ---- Load demo data into chart ----
    QVector<double> time, open, high, low, close, vol;
    auto now = std::chrono::system_clock::now();
    double price = 5200.0;
    for (int i = 0; i < 60; ++i) {
        double t = std::chrono::duration_cast<std::chrono::seconds>(
            (now + std::chrono::hours(i)).time_since_epoch()).count();
        time.append(t);
        double change = (std::sin(i * 0.4) * 30.0) + (std::cos(i * 0.15) * 15.0);
        double o = price;
        double c = price + change + (rand() % 20 - 10);
        double h = std::max(o, c) + std::abs(rand() % 10);
        double l = std::min(o, c) - std::abs(rand() % 10);
        open.append(o);
        high.append(h);
        low.append(l);
        close.append(c);
        vol.append(5000 + rand() % 10000);
        price = c;
    }
    m_chartPanel->loadCandlestickData(time, open, high, low, close, vol);
}

void MainWindow::switchPage(int index) {
    for (int i = 0; i < m_navBtns.size(); ++i) {
        m_navBtns[i]->setChecked(i == index);
    }
    m_stack->setCurrentIndex(index);
}

void MainWindow::setupTheme() {
    qApp->setStyleSheet(R"(
        /* ---- Global Dark Theme ---- */
        QMainWindow, QWidget {
            background-color: #1a1a1a;
            color: #cccccc;
            font-family: "Segoe UI", "Microsoft YaHei", sans-serif;
            font-size: 12px;
        }

        /* ---- Navbar ---- */
        QFrame#navbar {
            background-color: #111111;
            border-right: none;
        }

        QPushButton#navBtn {
            background-color: transparent;
            color: #888;
            border: none;
            border-radius: 6px;
            font-size: 10px;
            text-align: center;
        }
        QPushButton#navBtn:hover {
            background-color: #2a2a2a;
            color: #ccc;
        }
        QPushButton#navBtn:checked {
            background-color: #1a3a2a;
            color: #26a65b;
            font-weight: bold;
        }

        /* ---- Splitter ---- */
        QSplitter::handle {
            background-color: #333;
            height: 2px;
        }

        /* ---- Group Box ---- */
        QGroupBox {
            border: 1px solid #333;
            border-radius: 4px;
            margin-top: 12px;
            padding-top: 10px;
            font-weight: bold;
            color: #aaa;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 6px;
        }

        /* ---- Scrollbar ---- */
        QScrollBar:vertical {
            background: #1a1a1a;
            width: 8px;
        }
        QScrollBar::handle:vertical {
            background: #444;
            border-radius: 4px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover { background: #555; }

        /* ---- Status Bar ---- */
        QStatusBar { background-color: #111; color: #777; border-top: 1px solid #222; }
    )");
}
