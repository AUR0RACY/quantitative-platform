#include <QApplication>
#include <QMessageBox>
#include "ui/mainwindow.hpp"
#include "data/database.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Quantitative Platform");
    app.setApplicationVersion("0.2.0");

    // ---- Initialize database ----
    try {
        qp::data::Database db("quant.db");
        db.migrate();
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Database Error", e.what());
        return 1;
    }

    MainWindow window;
    window.show();

    return app.exec();
}
