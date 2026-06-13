#include "chart_panel.hpp"
#include <QVBoxLayout>
#include <QSplitter>

ChartPanel::ChartPanel(QWidget *parent) : QWidget(parent) {
    auto *splitter = new QSplitter(Qt::Vertical, this);

    // --- Price chart ---
    m_chart = new QCustomPlot;
    m_chart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    m_chart->axisRect()->setupFullAxesBox();
    m_chart->xAxis->setTickLabels(true);
    m_chart->yAxis->setLabel("Price");
    m_chart->legend->setVisible(true);
    m_chart->legend->setBrush(QBrush(QColor(255,255,255,200)));

    // --- Volume chart ---
    m_volumeChart = new QCustomPlot;
    m_volumeChart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_volumeChart->axisRect()->setupFullAxesBox();
    m_volumeChart->xAxis->setTickLabels(true);
    m_volumeChart->yAxis->setLabel("Volume");

    // Sync x-axes
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

void ChartPanel::loadCandlestickData(
    const QVector<double> &time,
    const QVector<double> &open,
    const QVector<double> &high,
    const QVector<double> &low,
    const QVector<double> &close,
    const QVector<double> &volume)
{
    Q_UNUSED(open)
    Q_UNUSED(low)
    m_chart->clearPlottables();
    m_volumeChart->clearPlottables();

    // Close price line with fill
    auto *priceLine = m_chart->addGraph();
    priceLine->setData(time, close);
    QPen pricePen(QColor(38, 166, 91), 2);
    priceLine->setPen(pricePen);
    priceLine->setName("Close");

    // High band (semi-transparent)
    auto *highBand = m_chart->addGraph();
    highBand->setData(time, high);
    highBand->setPen(QPen(QColor(38, 166, 91, 80), 1, Qt::DotLine));
    highBand->setName("High");
    highBand->setChannelFillGraph(priceLine);

    auto *lowBand = m_chart->addGraph();
    lowBand->setData(time, low);
    lowBand->setPen(QPen(QColor(240, 75, 75, 80), 1, Qt::DotLine));
    lowBand->setName("Low");

    // Volume bars
    auto *volBars = new QCPBars(m_volumeChart->xAxis, m_volumeChart->yAxis);
    volBars->setData(time, volume);
    volBars->setWidth(3600 * 20);
    volBars->setPen(Qt::NoPen);

    for (int i = 0; i < time.size(); ++i) {
        if (close[i] >= open[i]) {
            volBars->setBrush(QColor(38, 166, 91, 120));
        } else {
            volBars->setBrush(QColor(240, 75, 75, 120));
        }
    }

    m_chart->rescaleAxes();
    m_volumeChart->rescaleAxes();
    m_chart->replot(QCustomPlot::rpQueuedReplot);
    m_volumeChart->replot(QCustomPlot::rpQueuedReplot);
}
