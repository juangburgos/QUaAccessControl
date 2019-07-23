#include "quarolewidgetedit.h"
#include "ui_quarolewidgetedit.h"

#include <QMessageBox>
#include <QMetaEnum>

#include <QUaAccessControl>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>

QUaRoleWidgetEdit::QUaRoleWidgetEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaRoleWidgetEdit)
{
    ui->setupUi(this);
	m_loggedUser = nullptr;
	// forward signals
	QObject::connect(ui->pushButtonDelete, &QPushButton::clicked, this, &QUaRoleWidgetEdit::deleteClicked);
	// setup table
	// setup users table model
	m_modelUsers.setColumnCount((int)Headers::Invalid);
	QStringList userHeaders;
	for (int i = (int)Headers::Name; i < (int)Headers::Invalid; i++)
	{
		userHeaders << QString(QMetaEnum::fromType<Headers>().valueToKey(i));
	}
	m_modelUsers.setHorizontalHeaderLabels(userHeaders);
	// setup user sort filter
	m_proxyUsers.setSourceModel(&m_modelUsers);
	// setup user table
	ui->tableViewUsers->setModel(&m_proxyUsers);
	ui->tableViewUsers->setAlternatingRowColors(true);
	ui->tableViewUsers->horizontalHeader()->setStretchLastSection(true);
	ui->tableViewUsers->verticalHeader()->setVisible(false);
	ui->tableViewUsers->setSortingEnabled(true);
	ui->tableViewUsers->sortByColumn((int)Headers::Name, Qt::SortOrder::AscendingOrder);
	ui->tableViewUsers->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableViewUsers->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableViewUsers->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

QUaRoleWidgetEdit::~QUaRoleWidgetEdit()
{
    delete ui;
}

bool QUaRoleWidgetEdit::isRoleNameReadOnly() const
{
	return ui->lineEditRoleName->isReadOnly();
}

void QUaRoleWidgetEdit::setRoleNameReadOnly(const bool & readOnly)
{
	ui->lineEditRoleName->setReadOnly(readOnly);
}

bool QUaRoleWidgetEdit::areActionsVisible() const
{
	return ui->frameEditActions->isEnabled();
}

void QUaRoleWidgetEdit::setActionsVisible(const bool & isVisible)
{
	ui->frameEditActions->setEnabled(isVisible);
	ui->frameEditActions->setVisible(isVisible);	
}

bool QUaRoleWidgetEdit::isUserListVisible() const
{
	return ui->tableViewUsers->isEnabled();
}

void QUaRoleWidgetEdit::setUserListVisible(const bool & isVisible)
{
	ui->tableViewUsers->setEnabled(isVisible);
	ui->tableViewUsers->setVisible(isVisible);
	ui->labelUsers->setEnabled(isVisible);
	ui->labelUsers->setVisible(isVisible);
}

QString QUaRoleWidgetEdit::roleName() const
{
	return ui->lineEditRoleName->text();
}

void QUaRoleWidgetEdit::setRoleName(const QString & strRoleName)
{
	ui->lineEditRoleName->setText(strRoleName);
}

QStringList QUaRoleWidgetEdit::users() const
{
	QStringList retList;
	// get parent to add rows as children
	auto parent = m_modelUsers.invisibleRootItem();
	for (int row = 0; row < parent->rowCount(); row++)
	{
		auto item = m_modelUsers.item(row, (int)Headers::Name);
		Q_CHECK_PTR(item);
		retList << item->text();
	}
	return retList;
}

void QUaRoleWidgetEdit::setUsers(const QStringList & listUsers)
{
	// cleanup first
	m_modelUsers.removeRows(0, m_modelUsers.rowCount());
	for (auto userName : listUsers)
	{
		this->addUser(userName);
	}
}

void QUaRoleWidgetEdit::addUser(const QString & strUserName)
{
	// get parent to add rows as children
	auto parent = m_modelUsers.invisibleRootItem();
	auto row    = parent->rowCount();
	// name column
	auto iName = new QStandardItem(strUserName);
	// add
	parent->setChild(row, (int)Headers::Name, iName);
}

void QUaRoleWidgetEdit::removeUser(const QString & strUserName)
{
	auto listMatches = m_modelUsers.findItems(strUserName);
	Q_ASSERT(listMatches.count() <= 1);
	QStandardItem * match = listMatches.count() == 1 ? listMatches.at(0) : nullptr;
	Q_CHECK_PTR(match);
	if (!match)
	{
		return;
	}
	// remove from table
	m_modelUsers.removeRows(match->index().row(), 1);
}

void QUaRoleWidgetEdit::on_loggedUserChanged(QUaUser * user)
{
	m_loggedUser = user;
	// update table filter
	m_proxyUsers.resetFilter();
}