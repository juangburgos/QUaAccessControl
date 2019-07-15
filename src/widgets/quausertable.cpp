#include "quausertable.h"
#include "ui_quausertable.h"

QUaUserTable::QUaUserTable(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaUserTable)
{
    ui->setupUi(this);
}

QUaUserTable::~QUaUserTable()
{
    delete ui;
}

void QUaUserTable::on_pushButtonAdd_clicked()
{

}
