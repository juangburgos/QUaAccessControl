#include "qaddockwidgetwrapper.h"
#include "ui_qaddockwidgetwrapper.h"

QAdDockWidgetWrapper::QAdDockWidgetWrapper(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QAdDockWidgetWrapper)
{
    ui->setupUi(this);
	// forward signals
	QObject::connect(ui->pushButtonConfig, &QPushButton::clicked, this, &QAdDockWidgetWrapper::configClicked);
}

QAdDockWidgetWrapper::~QAdDockWidgetWrapper()
{
    delete ui;
}

bool QAdDockWidgetWrapper::isEditBarVisible() const
{
	return ui->frameEdit->isVisible();
}

void QAdDockWidgetWrapper::setEditBarVisible(const bool & isVisible)
{
	ui->frameEdit->setVisible(isVisible);
}

QWidget * QAdDockWidgetWrapper::widget() const
{
	// find the widget and return a reference to it
	return this->findChild<QWidget*>("wrappedwidget");
}

void QAdDockWidgetWrapper::setWidget(QWidget * w)
{
	// check pointer
	Q_CHECK_PTR(w);
	if (!w)
	{
		return;
	}
	// check if has widget already
	Q_ASSERT(!this->widget());
	if (this->widget())
	{
		return;
	}
	// take ownership
	w->setParent(this);
	// set object name in order to be able to retrieve later
	w->setObjectName("wrappedwidget");
	// put the widget in the layout
	ui->verticalLayout->insertWidget(1, w);
	//ui->verticalLayout->setSizeConstraint(QLayout::SetFixedSize);
}
