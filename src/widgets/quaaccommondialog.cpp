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
	// take ownership
	w->setParent(this);
	// set object name in order to be able to retrieve later
	w->setObjectName("diagwidget");
	// put the widget in the layout
	ui->verticalLayout->insertWidget(0, w);
	ui->verticalLayout->setSizeConstraint(QLayout::SetFixedSize);
}
