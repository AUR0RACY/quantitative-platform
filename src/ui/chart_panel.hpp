#pragma once

#include <QWidget>
#include "qcustomplot.h"

struct CandlestickData {
    QVector<double> time, open, high, low, close, volume;
};

class ChartPanel : public QWidget {
    Q_OBJECT
public:
    explicit ChartPanel(QWidget *parent = nullptr);

    void loadData(const CandlestickData &data);
    void addIndicator(const QString &name, const QVector<double> &time, const QVector<double> &values,
                      QColor color = Qt::yellow);
    void clearIndicators();

private:
    void drawCandles(const CandlestickData &d);
    void drawVolume(const CandlestickData &d);

    QCustomPlot *m_chart;
    QCustomPlot *m_volumeChart;
    CandlestickData m_data;
    QVector<QCPGraph*> m_indicators;
};
