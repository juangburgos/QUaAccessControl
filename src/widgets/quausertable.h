#ifndef QUAUSERTABLE_H
#define QUAUSERTABLE_H

#include <QWidget>

namespace Ui {
class QUaUserTable;
}

class QUaUserTable : public QWidget
{
    Q_OBJECT

public:
    explicit QUaUserTable(QWidget *parent = nullptr);
    ~QUaUserTable();

	// TODO : pass in logged user (permissions, disable everything if no logged user)

	// TODO : pass in access control (need user list, role list for combo)

private slots:
    void on_pushButtonAdd_clicked();

private:
    Ui::QUaUserTable *ui;
};

#endif // QUAUSERTABLE_H
