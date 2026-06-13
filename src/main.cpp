#include <QApplication>
#include "ui/mainwindow.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Quantitative Platform");
    app.setApplicationVersion("0.1.0");

    MainWindow window;
    window.show();

    return app.exec();
}
