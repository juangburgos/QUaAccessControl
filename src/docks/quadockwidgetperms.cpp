#include "quadockwidgetperms.h"
#include "ui_quadockwidgetperms.h"

#include <QUaAccessControl>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>
#include <QUaPermissionsList>


int QUaDockWidgetPerms::PointerRole = Qt::UserRole + 1;

QUaDockWidgetPerms::QUaDockWidgetPerms(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaDockWidgetPerms)
{
    ui->setupUi(this);
	m_deleting = false;
	// hide stuff
	ui->widgetPermsView->setActionsVisible(false);
	ui->widgetPermsView->setIdVisible(false);
	ui->widgetPermsView->setAccessReadOnly(true);
	// events
	QObject::connect(ui->comboBoxPermissions, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QUaDockWidgetPerms::on_currentIndexChanged);
}

QUaDockWidgetPerms::~QUaDockWidgetPerms()
{
	// disable old connections
	while (m_connections.count() > 0)
	{
		QObject::disconnect(m_connections.takeFirst());
	}
	m_deleting = true;
    delete ui;
}

void QUaDockWidgetPerms::setComboModel(QSortFilterProxyModel * proxy)
{
	Q_CHECK_PTR(proxy);
	// setup combo
	ui->comboBoxPermissions->setModel(proxy);
	ui->comboBoxPermissions->setEditable(true);
	// setup completer
	QCompleter *completer = new QCompleter(ui->comboBoxPermissions);
	completer->setModel(proxy);
	completer->setFilterMode(Qt::MatchContains);
	ui->comboBoxPermissions->setCompleter(completer);
	ui->comboBoxPermissions->setInsertPolicy(QComboBox::NoInsert);
}

QSortFilterProxyModel * QUaDockWidgetPerms::comboModel() const
{
	return dynamic_cast<QSortFilterProxyModel*>(ui->comboBoxPermissions->model());
}

QUaPermissions * QUaDockWidgetPerms::permissions() const
{
	return ui->comboBoxPermissions->currentData(QUaDockWidgetPerms::PointerRole).value<QUaPermissions*>();
}

void QUaDockWidgetPerms::setPermissions(const QUaPermissions * permissions)
{
	QString strId = "";
	if (permissions)
	{
		strId = permissions->getId();
	}
	auto index = ui->comboBoxPermissions->findText(strId);
	Q_ASSERT(index >= 0);
	// NOTE : setCurrentText does not work, it does not hold the pointer (userData)
	ui->comboBoxPermissions->setCurrentIndex(index);
	Q_ASSERT(ui->comboBoxPermissions->currentData(QUaDockWidgetPerms::PointerRole).value<QUaPermissions*>() == permissions);
	// populate ui->widgetPermsView
	this->bindWidgetPermissionsEdit(permissions);
}

void QUaDockWidgetPerms::on_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	this->bindWidgetPermissionsEdit(this->permissions());
}

void QUaDockWidgetPerms::clearWidgetPermissionsEdit()
{
	ui->widgetPermsView->setId("");
	ui->widgetPermsView->setRoleAccessMap(QUaRoleAccessMap());
	ui->widgetPermsView->setUserAccessMap(QUaUserAccessMap());
	ui->widgetPermsView->setEnabled(false);
}

void QUaDockWidgetPerms::bindWidgetPermissionsEdit(const QUaPermissions * perms)
{
	if (!perms)
	{
		return;
	}
	// get access control
	auto ac = perms->list()->accessControl();
	Q_CHECK_PTR(ac);
	// disable old connections
	while (m_connections.count() > 0)
	{
		QObject::disconnect(m_connections.takeFirst());
	}
	// bind common
	m_connections <<
	QObject::connect(perms, &QObject::destroyed, this,
	[this]() {
		if (m_deleting)
		{
			return;
		}
		// clear widget
		this->clearWidgetPermissionsEdit();
	});

	// roles (all)
	QUaRoleAccessMap mapRoles;
	auto roles = ac->roles()->roles();
	for (auto role : roles)
	{
		mapRoles[role->getName()] = {
			perms->canRoleRead(role),
			perms->canRoleWrite(role)
		};
	}
	ui->widgetPermsView->setRoleAccessMap(mapRoles);

	// users (all)
	QUaUserAccessMap mapUsers;
	auto users = ac->users()->users();
	for (auto user : users)
	{
		mapUsers[user->getName()] = {
			perms->canUserReadDirectly(user),
			perms->canUserWriteDirectly(user),
			perms->canRoleRead(user->role()),
			perms->canRoleWrite(user->role())
		};
	}
	ui->widgetPermsView->setUserAccessMap(mapUsers);

	// role updates
	auto updateRole = [this, perms](QUaRole * role) {
		// role table
		ui->widgetPermsView->updateRoleAccess(
			role->getName(),
			{ perms->canRoleRead(role), perms->canRoleWrite(role) }
		);
		// user table
		auto users = role->users();
		for (auto user : users)
		{
			ui->widgetPermsView->updateUserAccess(
				user->getName(),
				{
				perms->canUserReadDirectly(user),
				perms->canUserWriteDirectly(user),
				perms->canRoleRead(user->role()),
				perms->canRoleWrite(user->role())
				}
			);
		}
	};
	m_connections << QObject::connect(perms, &QUaPermissions::canReadRoleAdded   , this, updateRole, Qt::QueuedConnection);
	m_connections << QObject::connect(perms, &QUaPermissions::canReadRoleRemoved , this, updateRole, Qt::QueuedConnection);
	m_connections << QObject::connect(perms, &QUaPermissions::canWriteRoleAdded  , this, updateRole, Qt::QueuedConnection);
	m_connections << QObject::connect(perms, &QUaPermissions::canWriteRoleRemoved, this, updateRole, Qt::QueuedConnection);

	// user updates
	auto updateUser = [this, perms](QUaUser * user) {
		ui->widgetPermsView->updateUserAccess(
			user->getName(), 
			{
			perms->canUserReadDirectly (user),
			perms->canUserWriteDirectly(user),
			perms->canRoleRead (user->role()),
			perms->canRoleWrite(user->role())
			}
		);
	};
	m_connections << QObject::connect(perms, &QUaPermissions::canReadUserAdded   , this, updateUser, Qt::QueuedConnection);
	m_connections << QObject::connect(perms, &QUaPermissions::canReadUserRemoved , this, updateUser, Qt::QueuedConnection);
	m_connections << QObject::connect(perms, &QUaPermissions::canWriteUserAdded  , this, updateUser, Qt::QueuedConnection);
	m_connections << QObject::connect(perms, &QUaPermissions::canWriteUserRemoved, this, updateUser, Qt::QueuedConnection);

	// user changes role
	for (auto user : users)
	{
		m_connections << QObject::connect(user, &QUaUser::roleChanged, this, 
		[updateUser, user]() {
			updateUser(user);
		}, Qt::QueuedConnection);
	}

	// on user or role added/removed
	auto resetPermsWidget = [this, perms]() {
		this->bindWidgetPermissionsEdit(perms);
	};
	// NOTE : queued to wait until user/role has name or has actually been deleted
	m_connections << QObject::connect(ac->roles(), &QUaRoleList::roleAdded  , perms, resetPermsWidget, Qt::QueuedConnection);
	m_connections << QObject::connect(ac->roles(), &QUaRoleList::roleRemoved, perms, resetPermsWidget, Qt::QueuedConnection);
	m_connections << QObject::connect(ac->users(), &QUaUserList::userAdded  , perms, resetPermsWidget, Qt::QueuedConnection);
	m_connections << QObject::connect(ac->users(), &QUaUserList::userRemoved, perms, resetPermsWidget, Qt::QueuedConnection);
}
