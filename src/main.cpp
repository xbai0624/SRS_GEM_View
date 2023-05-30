/*
 * test ssp decoder
 */

#include <QApplication>
//#include <TApplication.h>

#include "Viewer.h"

////////////////////////////////////////////////////////////////
// An example for how to use the raw event decoder

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    QApplication app(argc, argv);
    //TApplication root_app("root", &argc, argv);

    Viewer *viewer = new Viewer();
    viewer -> resize(1200, 550);
    viewer -> show();

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
    return app.exec();
}
