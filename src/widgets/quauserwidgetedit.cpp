#include "quauserwidgetedit.h"
#include "ui_quauserwidgetedit.h"

QUaUserWidgetEdit::QUaUserWidgetEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaUserWidgetEdit)
{
    ui->setupUi(this);
}

QUaUserWidgetEdit::~QUaUserWidgetEdit()
{
    delete ui;
}

bool QUaUserWidgetEdit::isRoleVisible() const
{
	return ui->comboBoxRole->isEnabled();
}

void QUaUserWidgetEdit::setRoleVisible(const bool & isVisible)
{
	ui->comboBoxRole->setEnabled(isVisible);
	ui->comboBoxRole->setVisible(isVisible);
	ui->labelRole->setEnabled(isVisible);
	ui->labelRole->setVisible(isVisible);
}

bool QUaUserWidgetEdit::isHashVisible() const
{
	return ui->plainTextEditHash->isEnabled();
}

void QUaUserWidgetEdit::setHashVisible(const bool & isVisible)
{
	ui->plainTextEditHash->setEnabled(isVisible);
	ui->plainTextEditHash->setVisible(isVisible);
	ui->labelHash->setEnabled(isVisible);
	ui->labelHash->setVisible(isVisible);
}

QString QUaUserWidgetEdit::userName() const
{
	return ui->lineEditUserName->text();
}

void QUaUserWidgetEdit::setUserName(const QString & strUserName)
{
	ui->lineEditUserName->setText(strUserName);
}

QString QUaUserWidgetEdit::password() const
{
	return ui->lineEditPass->text();
}

void QUaUserWidgetEdit::setPassword(const QString & strPassword)
{
	ui->lineEditPass->setText(strPassword);
}

QString QUaUserWidgetEdit::hash() const
{
	return ui->plainTextEditHash->toPlainText();
}

void QUaUserWidgetEdit::setHash(const QString & strHexHash)
{
	ui->plainTextEditHash->setPlainText(strHexHash);
}
