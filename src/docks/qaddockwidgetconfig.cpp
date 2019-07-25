#include "qaddockwidgetconfig.h"
#include "ui_qaddockwidgetconfig.h"

QAdDockWidgetConfig::QAdDockWidgetConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QAdDockWidgetConfig)
{
    ui->setupUi(this);
}

QAdDockWidgetConfig::~QAdDockWidgetConfig()
{
    delete ui;
}

QWidget * QAdDockWidgetConfig::configWidget() const
{
	// find the widget and return a reference to it
	return this->findChild<QWidget*>("configwidget");
}

void QAdDockWidgetConfig::setConfigWidget(QWidget * w)
{
	// check pointer
	Q_CHECK_PTR(w);
	if (!w)
	{
		return;
	}
	// check if has widget already
	Q_ASSERT(!this->configWidget());
	if (this->configWidget())
	{
		return;
	}
	// take ownership
	w->setParent(this);
	// set object name in order to be able to retrieve later
	w->setObjectName("configwidget");
	// put the widget in the tab widget
	ui->tabWidget->addTab(w, tr("Config"));
}

QWidget * QAdDockWidgetConfig::permissionsWidget() const
{
	// find the widget and return a reference to it
	return this->findChild<QWidget*>("permswidget");
}

void QAdDockWidgetConfig::setPermissionsWidget(QWidget * w)
{
	// check pointer
	Q_CHECK_PTR(w);
	if (!w)
	{
		return;
	}
	// check if has widget already
	Q_ASSERT(!this->configWidget());
	if (this->configWidget())
	{
		return;
	}
	// take ownership
	w->setParent(this);
	// set object name in order to be able to retrieve later
	w->setObjectName("permswidget");
	// put the widget in the tab widget
	ui->tabWidget->addTab(w, tr("Permissions"));
}
