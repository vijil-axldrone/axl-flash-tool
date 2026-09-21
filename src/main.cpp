#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // Force XCB backend on Linux to fix QComboBox closing immediately on Wayland
#ifdef Q_OS_LINUX
    qputenv("QT_QPA_PLATFORM", "xcb");
#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
