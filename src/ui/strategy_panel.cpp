#include "strategy_panel.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QHeaderView>
#include <QFont>

StrategyPanel::StrategyPanel(QWidget *parent) : QWidget(parent) {
    auto *splitter = new QSplitter(Qt::Vertical, this);

    // --- Top: Code editor ---
    auto *editorWidget = new QWidget;
    auto *edLayout = new QVBoxLayout(editorWidget);
    edLayout->setContentsMargins(0, 0, 0, 0);

    auto *toolbar = new QHBoxLayout;
    auto *titleLabel = new QLabel("C++ Strategy Editor");
    titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; padding: 4px;");
    m_compileBtn = new QPushButton("Compile (MSVC)");
    m_compileBtn->setFixedWidth(160);
    m_compileBtn->setStyleSheet(
        "QPushButton { background-color: #1a7f3f; color: white; border: none; "
        "border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #22994d; }");
    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setStyleSheet("color: #888; padding: 4px;");

    toolbar->addWidget(titleLabel);
    toolbar->addStretch();
    toolbar->addWidget(m_statusLabel);
    toolbar->addWidget(m_compileBtn);
    edLayout->addLayout(toolbar);

    m_editor = new QTextEdit;
    QFont monoFont("Consolas", 11);
    m_editor->setFont(monoFont);
    m_editor->setPlaceholderText(
        "// Write your C++ strategy here...\n"
        "// Implement IStrategy: init(), on_bar(), params(), set_param()\n\n"
        "#include \"engine/strategy_interface.hpp\"\n\n"
        "class MyStrategy : public qp::engine::IStrategy {\n"
        "    // ...\n"
        "};\n\n"
        "extern \"C\" __declspec(dllexport) qp::engine::IStrategy* create_strategy() {\n"
        "    return new MyStrategy();\n"
        "}\n"
        "extern \"C\" __declspec(dllexport) void destroy_strategy(qp::engine::IStrategy* s) {\n"
        "    delete s;\n"
        "}");
    m_editor->setStyleSheet(
        "QTextEdit { background-color: #1e1e1e; color: #d4d4d4; border: 1px solid #333; "
        "border-radius: 4px; padding: 8px; }");

    edLayout->addWidget(m_editor);
    splitter->addWidget(editorWidget);

    // --- Bottom: Parameters ---
    m_paramTable = new QTableWidget(0, 4);
    m_paramTable->setHorizontalHeaderLabels({"Name", "Value", "Min", "Max"});
    m_paramTable->horizontalHeader()->setStretchLastSection(true);
    m_paramTable->setStyleSheet(
        "QTableWidget { background-color: #252525; color: #ccc; gridline-color: #333; }"
        "QHeaderView::section { background-color: #333; color: #ccc; padding: 4px; }");
    splitter->addWidget(m_paramTable);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(splitter);

    connect(m_compileBtn, &QPushButton::clicked, [this]() {
        m_statusLabel->setText("Compiling...");
        m_statusLabel->setStyleSheet("color: #f0c040; padding: 4px;");
        emit compileRequested(m_editor->toPlainText());
    });
}
