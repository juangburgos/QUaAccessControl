#include "quauserwidgetedit.h"
#include "ui_quauserwidgetedit.h"

#include <QUaRoleList>
#include <QUaRole>

int QUaUserWidgetEdit::PointerRole = Qt::UserRole + 1;

QUaUserWidgetEdit::QUaUserWidgetEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaUserWidgetEdit)
{
    ui->setupUi(this);
	// forward signals
	QObject::connect(ui->pushButtonDelete, &QPushButton::clicked, this, &QUaUserWidgetEdit::deleteClicked);
	QObject::connect(ui->pushButtonApply , &QPushButton::clicked, this, &QUaUserWidgetEdit::applyClicked );
	// setup combo model
	m_proxyCombo.setSourceModel(&m_modelCombo);
	// setup combo
	ui->comboBoxRole->setModel(&m_proxyCombo);
	ui->comboBoxRole->setEditable(true);
	// setup completer
	QCompleter *completer = new QCompleter(ui->comboBoxRole);
	completer->setModel(&m_proxyCombo);
	completer->setFilterMode(Qt::MatchContains);
	ui->comboBoxRole->setCompleter(completer);
	ui->comboBoxRole->setInsertPolicy(QComboBox::NoInsert);
}

QUaUserWidgetEdit::~QUaUserWidgetEdit()
{
    delete ui;
}

bool QUaUserWidgetEdit::isUserNameReadOnly() const
{
	return ui->lineEditUserName->isReadOnly();
}

void QUaUserWidgetEdit::setUserNameReadOnly(const bool & readOnly)
{
	ui->lineEditUserName->setReadOnly(readOnly);
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

bool QUaUserWidgetEdit::isRoleReadOnly() const
{
	return ui->comboBoxRole->isReadOnly();
}

void QUaUserWidgetEdit::setRoleReadOnly(const bool & readOnly)
{
	ui->comboBoxRole->setReadOnly(readOnly);
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

bool QUaUserWidgetEdit::isPasswordVisible() const
{
	return ui->lineEditPass->isEnabled();
}

void QUaUserWidgetEdit::setPasswordVisible(const bool & isVisible)
{
	ui->lineEditPass->setEnabled(isVisible);
	ui->lineEditPass->setVisible(isVisible);
	ui->labelPassword->setEnabled(isVisible);
	ui->labelPassword->setVisible(isVisible);
}

bool QUaUserWidgetEdit::areActionsVisible() const
{
	return ui->frameEditActions->isEnabled();
}

void QUaUserWidgetEdit::setActionsVisible(const bool & isVisible)
{
	ui->frameEditActions->setEnabled(isVisible);
	ui->frameEditActions->setVisible(isVisible);
}

bool QUaUserWidgetEdit::isDeleteVisible() const
{
	return ui->pushButtonDelete->isEnabled();
}

void QUaUserWidgetEdit::setDeleteVisible(const bool & isVisible)
{
	ui->pushButtonDelete->setEnabled(isVisible);
	ui->pushButtonDelete->setVisible(isVisible);
}

void QUaUserWidgetEdit::setRoleList(const QUaRoleList * listRoles)
{
	// disable connections
	while (m_connections.count() > 0)
	{
		QObject::disconnect(m_connections.takeFirst());
	}
	// re-set model for comboboxes
	m_modelCombo.clear();
	// add empty/undefined
	auto parent = m_modelCombo.invisibleRootItem();
	auto row    = parent->rowCount();
	auto col    = 0;
	auto iInvalidParam = new QStandardItem("");
	parent->setChild(row, col, iInvalidParam);
	iInvalidParam->setData(QVariant::fromValue(nullptr), QUaUserWidgetEdit::PointerRole);
	// add all existing roles
	auto roles = listRoles->roles();
	for (int i = 0; i < roles.count(); i++)
	{
		auto role = roles.at(i);
		row = parent->rowCount();
		auto iRole = new QStandardItem(role->getName());
		parent->setChild(row, col, iRole);
		iRole->setData(QVariant::fromValue(role), QUaUserWidgetEdit::PointerRole);
		// subscribe to destroyed
		m_connections << QObject::connect(role, &QObject::destroyed, this,
			[this, iRole]() {
			Q_CHECK_PTR(iRole);
			// remove from model
			m_modelCombo.removeRows(iRole->index().row(), 1);
		});
	}
	// subscribe to role created
	m_connections << QObject::connect(listRoles, &QUaRoleList::roleAdded, this,
	[this, col](QUaRole * role) {
		auto parent = m_modelCombo.invisibleRootItem();
		auto row    = parent->rowCount();
		auto iRole  = new QStandardItem(role->getName());
		parent->setChild(row, col, iRole);
		iRole->setData(QVariant::fromValue(role), QUaUserWidgetEdit::PointerRole);
		// subscribe to destroyed
		m_connections << QObject::connect(role, &QObject::destroyed, this,
		[this, iRole]() {
			Q_CHECK_PTR(iRole);
			// remove from model
			m_modelCombo.removeRows(iRole->index().row(), 1);
		});
	});
}

QString QUaUserWidgetEdit::userName() const
{
	return ui->lineEditUserName->text();
}

void QUaUserWidgetEdit::setUserName(const QString & strUserName)
{
	ui->lineEditUserName->setText(strUserName);
}

QUaRole * QUaUserWidgetEdit::role() const
{
	return ui->comboBoxRole->currentData(QUaUserWidgetEdit::PointerRole).value<QUaRole*>();;
}

void QUaUserWidgetEdit::setRole(const QUaRole * role)
{
	QString strRole = "";
	if (role)
	{
		strRole = role->getName();
	}
	auto index = ui->comboBoxRole->findText(strRole);
	Q_ASSERT(index >= 0);
	// NOTE : setCurrentText does not work, it does not hold the pointer (userData)
	ui->comboBoxRole->setCurrentIndex(index);
	Q_ASSERT(ui->comboBoxRole->currentData(QUaUserWidgetEdit::PointerRole).value<QUaRole*>() == role);
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
