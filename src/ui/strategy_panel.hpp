#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>

class StrategyPanel : public QWidget {
    Q_OBJECT
public:
    explicit StrategyPanel(QWidget *parent = nullptr);
    void setCompileCallback(std::function<void(const QString&)> cb) { m_compileCallback = cb; }

signals:
    void compileRequested(const QString &code);

private:
    QTextEdit   *m_editor;
    QTableWidget *m_paramTable;
    QPushButton *m_compileBtn;
    QLabel      *m_statusLabel;
    std::function<void(const QString&)> m_compileCallback;
};
