#ifndef QUADOCKWIDGETPERMS_H
#define QUADOCKWIDGETPERMS_H

#include <QWidget>

namespace Ui {
class QUaDockWidgetPerms;
}

class QUaDockWidgetPerms : public QWidget
{
    Q_OBJECT

public:
    explicit QUaDockWidgetPerms(QWidget *parent = nullptr);
    ~QUaDockWidgetPerms();

private:
    Ui::QUaDockWidgetPerms *ui;
};

#endif // QUADOCKWIDGETPERMS_H
