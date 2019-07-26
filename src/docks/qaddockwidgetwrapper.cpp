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
	// adjust dialog's geometry
	auto geo = this->geometry();
	geo.setWidth (1.1 * w->width ());
	geo.setHeight(1.1 * w->height());
	this->setGeometry(geo);
	// if parent defined, move to center of parent
	auto parent = dynamic_cast<QWidget*>(this->parent());
	if (!parent)
	{
		return;
	}
	// get root parent
	while(parent)
	{
		auto newParent = dynamic_cast<QWidget*>(parent->parent());
		if (!newParent)
		{
			break;
		}
		parent = newParent;
	}
	this->move(
		(parent->width() / 2) - (geo.width() / 2),
		(parent->height() / 2) - (geo.height() / 2)
	);
}
