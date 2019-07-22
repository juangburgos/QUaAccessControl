#include "quaacfullui.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QUaAcFullUi w;
    w.showMaximized();

    return a.exec();
}
