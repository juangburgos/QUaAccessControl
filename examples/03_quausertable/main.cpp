#include "quausertabletestdialog.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QUaUserTableTestDialog w;
    w.show();

    return a.exec();
}
