#include "mainwindow.h"
#include "logging.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    Logging logging;
    qInstallMessageHandler(Logging::messageOutput);
    QLoggingCategory::installFilter(Logging::categoryFilter);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
