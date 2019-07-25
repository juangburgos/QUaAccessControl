#include "quaaccommondialog.h"
#include "ui_quaaccommondialog.h"

#include <QPushButton>

QUaAcCommonDialog::QUaAcCommonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QUaAcCommonDialog)
{
    ui->setupUi(this);
	ui->buttonBox->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	ui->buttonBox->button(QDialogButtonBox::StandardButton::Cancel)->setFocusPolicy(Qt::FocusPolicy::NoFocus);
}

QUaAcCommonDialog::~QUaAcCommonDialog()
{
    delete ui;
}

QWidget * QUaAcCommonDialog::widget() const
{
	// find the widget and return a reference to it
	return this->findChild<QWidget*>("diagwidget");
}

void QUaAcCommonDialog::setWidget(QWidget * w)
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
	// adjust widget to compensate for hidden widgets
	w->adjustSize();
	// take ownership
	w->setParent(this);
	// set object name in order to be able to retrieve later
	w->setObjectName("diagwidget");
	// put the widget in the layout
	ui->verticalLayout->insertWidget(0, w);
	// adjust dialog's geaometry
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
