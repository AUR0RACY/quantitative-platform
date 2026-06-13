#include "chart_panel.hpp"
#include <QVBoxLayout>
#include <QSplitter>
#include <cmath>

ChartPanel::ChartPanel(QWidget *parent) : QWidget(parent) {
    auto *splitter = new QSplitter(Qt::Vertical, this);

    // Price chart
    m_chart = new QCustomPlot;
    m_chart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    m_chart->axisRect()->setupFullAxesBox();
    m_chart->xAxis->setTickLabels(true);
    m_chart->yAxis->setLabel("Price");
    m_chart->legend->setVisible(true);
    m_chart->legend->setBrush(QBrush(QColor(40, 40, 40, 220)));
    m_chart->legend->setBorderPen(QPen(QColor(80, 80, 80)));

    // Volume chart
    m_volumeChart = new QCustomPlot;
    m_volumeChart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_volumeChart->axisRect()->setupFullAxesBox();
    m_volumeChart->xAxis->setTickLabels(true);
    m_volumeChart->yAxis->setLabel("Volume");

    connect(m_chart->xAxis, SIGNAL(rangeChanged(QCPRange)),
            m_volumeChart->xAxis, SLOT(setRange(QCPRange)));

    splitter->addWidget(m_chart);
    splitter->addWidget(m_volumeChart);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(splitter);
}

void ChartPanel::loadData(const CandlestickData &data) {
    m_data = data;
    drawCandles(data);
    drawVolume(data);
    m_chart->rescaleAxes();
    m_volumeChart->rescaleAxes();
    m_chart->replot(QCustomPlot::rpQueuedReplot);
    m_volumeChart->replot(QCustomPlot::rpQueuedReplot);
}

void ChartPanel::drawCandles(const CandlestickData &d) {
    m_chart->clearPlottables();
    m_indicators.clear();

    int n = d.time.size();
    if (n == 0) return;

    QPen greenPen(QColor(38, 166, 91));
    QPen redPen(QColor(240, 75, 75));
    QBrush greenBrush(QColor(38, 166, 91, 220));
    QBrush redBrush(QColor(240, 75, 75, 220));

    // Estimate bar width in x-axis units
    double barWidth = (n > 1) ? (d.time[n-1] - d.time[0]) / n * 0.6 : 3600;

    // Draw each candle: wick (line) + body (rectangle)
    for (int i = 0; i < n; ++i) {
        double x  = d.time[i];
        bool isUp = d.close[i] >= d.open[i];

        // Wick: high-low line
        auto *wick = new QCPItemLine(m_chart);
        wick->start->setCoords(x, d.low[i]);
        wick->end->setCoords(x, d.high[i]);
        wick->setPen(isUp ? greenPen : redPen);

        // Body: rectangle from open to close
        double top = std::max(d.open[i], d.close[i]);
        double bot = std::min(d.open[i], d.close[i]);
        double hw  = barWidth * 0.4;

        auto *body = new QCPItemRect(m_chart);
        body->topLeft->setCoords(x - hw, top);
        body->bottomRight->setCoords(x + hw, bot);
        body->setPen(isUp ? greenPen : redPen);
        body->setBrush(isUp ? greenBrush : redBrush);
    }

    // Close price line on top
    auto *closeLine = m_chart->addGraph();
    closeLine->setData(d.time, d.close);
    closeLine->setPen(QPen(QColor(255, 255, 255, 60), 1));
    closeLine->setName("Close");
}

void ChartPanel::drawVolume(const CandlestickData &d) {
    m_volumeChart->clearPlottables();

    auto *volBars = new QCPBars(m_volumeChart->xAxis, m_volumeChart->yAxis);
    volBars->setData(d.time, d.volume);
    double barWidth = (d.time.size() > 1) ? (d.time[d.time.size()-1] - d.time[0]) / d.time.size() * 0.8 : 3600;
    volBars->setWidth(barWidth);
    volBars->setPen(Qt::NoPen);

    for (int i = 0; i < d.time.size(); ++i) {
        volBars->setBrush(d.close[i] >= d.open[i]
            ? QColor(38, 166, 91, 100) : QColor(240, 75, 75, 100));
    }
}

void ChartPanel::addIndicator(const QString &name, const QVector<double> &time,
                                const QVector<double> &values, QColor color) {
    auto *graph = m_chart->addGraph();
    graph->setData(time, values);
    graph->setPen(QPen(color, 2, Qt::SolidLine));
    graph->setName(name);
    m_indicators.append(graph);
    m_chart->replot(QCustomPlot::rpQueuedReplot);
}

void ChartPanel::clearIndicators() {
    for (auto *g : m_indicators) m_chart->removeGraph(g);
    m_indicators.clear();
    m_chart->replot(QCustomPlot::rpQueuedReplot);
}
