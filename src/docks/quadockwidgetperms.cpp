#include "quadockwidgetperms.h"
#include "ui_quadockwidgetperms.h"

QUaDockWidgetPerms::QUaDockWidgetPerms(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaDockWidgetPerms)
{
    ui->setupUi(this);
	// hide stuff
	ui->widgetPermsView->setActionsVisible(false);
	ui->widgetPermsView->setIdVisible(false);
}

QUaDockWidgetPerms::~QUaDockWidgetPerms()
{
    delete ui;
}
