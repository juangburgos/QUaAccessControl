#include "quapermissionstabletestdialog.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QUaPermissionsTableTestDialog w;
    w.show();

    return a.exec();
}
