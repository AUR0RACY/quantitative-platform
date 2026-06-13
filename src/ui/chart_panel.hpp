#pragma once

#include <QWidget>
#include "qcustomplot.h"

class ChartPanel : public QWidget {
    Q_OBJECT
public:
    explicit ChartPanel(QWidget *parent = nullptr);

    void loadCandlestickData(const QVector<double> &time,
                             const QVector<double> &open,
                             const QVector<double> &high,
                             const QVector<double> &low,
                             const QVector<double> &close,
                             const QVector<double> &volume);

private:
    QCustomPlot *m_chart;
    QCustomPlot *m_volumeChart;
};
