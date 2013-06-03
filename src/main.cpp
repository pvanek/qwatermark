#include "qwatermark.h"

#include <QtGui>
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication::setApplicationName("QWatermark");
    QApplication::setApplicationVersion("0.1");
    QApplication::setOrganizationName("yarpen.cz");
    QApplication::setOrganizationDomain("yarpen.cz");

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/logo.png"));

    QWatermark w;
    w.show();
    return a.exec();
}
