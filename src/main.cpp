/*
 * test ssp decoder
 */

#include <QApplication>
#include <QFile>

#include "Viewer.h"

////////////////////////////////////////////////////////////////
// An example for how to use the raw event decoder

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    QApplication app(argc, argv);

    // load a style
    // see: https://stackoverflow.com/questions/4810729/qt-setstylesheet-from-a-resource-qss-file
    // and https://github.com/GTRONICK/QSS
    QFile styleFile("./third_party/style/MacOS.qss");
    styleFile.open(QFile::ReadOnly);
    QString style(styleFile.readAll());
    app.setStyleSheet(style);

    Viewer *viewer = new Viewer();
    viewer -> resize(1200, 700);
    viewer -> show();

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
    return app.exec();
}
