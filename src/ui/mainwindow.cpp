#include "mainwindow.hpp"
#include <QLabel>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Quantitative Platform");
    resize(1200, 800);

    // ---- Placeholder central widget ----
    auto *label = new QLabel(
        "Quantitative Platform v0.1.0\n\n"
        "Trailblazer-inspired, built on C++ & Qt6.\n\n"
        "Phase 1: Project scaffold — complete.\n"
        "Phase 2: Data layer (CTP + SQLite) — next.",
        this
    );
    label->setAlignment(Qt::AlignCenter);
    QFont font = label->font();
    font.setPointSize(16);
    label->setFont(font);
    setCentralWidget(label);

    // ---- Menu bar ----
    auto *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("E&xit", this, &QWidget::close);

    auto *viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction("Chart");

    auto *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("About");

    // ---- Status bar ----
    statusBar()->showMessage("Ready");
}
