#include "quaacfullui.h"
#include "ui_quaacfullui.h"

#include <QLayout>

#include <QUaAccessControl>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>

#include <QUaUserTable>
#include <QUaRoleTable>
#include <QUaPermissionsTable>

#include <QUaUserWidgetEdit>
#include <QUaRoleWidgetEdit>
#include <QUaPermissionsWidgetEdit>

QUaAcFullUi::QUaAcFullUi(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QUaAcFullUi)
{
    ui->setupUi(this);
	// initial values
	m_deleting   = false;
	m_strSecret  = "my_secret";
	m_loggedUser = nullptr;
	// setup opc ua information model and server
	this->setupInfoModel();
	// create all widgets
	this->createWidgetsInDocks();
	// setup widgets
	this->setupUserWidgets();
	this->setupRoleWidgets();
	this->setupPermsWidgets();
	// handle user changed
	QObject::connect(this, &QUaAcFullUi::loggedUserChanged, this, &QUaAcFullUi::on_loggedUserChanged);
	
}

QUaAcFullUi::~QUaAcFullUi()
{
	m_deleting = true;
    delete ui;
}

void QUaAcFullUi::on_loggedUserChanged(QUaUser * user)
{
	// TODO :
	/*
	// set user edit widget permissions
	this->setWidgetUserEditPermissions(user);
	// update ui
	if (!user)
	{
		// logged out user
		ui->lineEditLoggedUser->setText("");
		ui->pushButtonLogout->setEnabled(false);
		// clear user edit widget
		this->clearWidgetUserEdit();
		return;
	}
	// logged in user
	ui->lineEditLoggedUser->setText(user->getName());
	ui->pushButtonLogout->setEnabled(true);
	*/
}

void QUaAcFullUi::setupInfoModel()
{
	// setup access control information model
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	auto ac = objsFolder->addChild<QUaAccessControl>();
	ac->setDisplayName("AccessControl");
	ac->setBrowseName("AccessControl");

	// disable anon login
	m_server.setAnonymousLoginAllowed(false);
	// define custom user validation 
	auto listUsers = ac->users();
	m_server.setUserValidationCallback(
		[listUsers](const QString &strUserName, const QString &strPassword) {
		auto user = listUsers->user(strUserName);
		if (!user)
		{
			return false;
		}
		return user->isPasswordValid(strPassword);
	});

	// setup auto add new user permissions
	QObject::connect(ac->users(), &QUaUserList::userAdded,
	[ac](QUaUser * user) {
		// return if user already has permissions
		if (user->hasPermissionsObject())
		{
			return;
		}
		// add user-only permissions
		auto permissionsList = ac->permissions();
		// create instance
		QString strPermId = QString("onlyuser_%1").arg(user->getName());
		permissionsList->addPermissions(strPermId);
		auto permissions = permissionsList->browseChild<QUaPermissions>(strPermId);
		Q_CHECK_PTR(permissions);
		// set user can read write
		permissions->addUserCanWrite(user);
		permissions->addUserCanRead (user);
		// set user's permissions
		user->setPermissionsObject(permissions);		
	});
	QObject::connect(ac->users(), &QUaUserList::userRemoved, 
	[ac](QUaUser * user) {
		// remove user-only permissions
		auto permissions = user->permissionsObject();
		if (!permissions)
		{
			return;
		}
		permissions->deleteLater();
	});
	// setup auto add new role permissions
	QObject::connect(ac->roles(), &QUaRoleList::roleAdded,
	[ac](QUaRole * role) {
		// return if role already has permissions
		if (role->hasPermissionsObject())
		{
			return;
		}
		// add role-only permissions
		auto permissionsList = ac->permissions();
		// create instance
		QString strPermId = QString("onlyrole_%1").arg(role->getName());
		permissionsList->addPermissions(strPermId);
		auto permissions = permissionsList->browseChild<QUaPermissions>(strPermId);
		Q_CHECK_PTR(permissions);
		// set role can read write
		permissions->addRoleCanWrite(role); // not used
		permissions->addRoleCanRead (role);
		// set role's permissions
		role->setPermissionsObject(permissions);
	});
	QObject::connect(ac->roles(), &QUaRoleList::roleRemoved, 
	[ac](QUaRole * role) {
		// remove role-only permissions
		auto permissions = role->permissionsObject();
		if (!permissions)
		{
			return;
		}
		permissions->deleteLater();
	});
	// setup auto add new role permissions
	QObject::connect(ac->permissions(), &QUaPermissionsList::permissionsAdded,
	[ac](QUaPermissions * perms) {
		// return if permissions already has permissions
		if (perms->hasPermissionsObject())
		{
			return;
		}
		// assign to itself
		perms->setPermissionsObject(perms);
	});

	// start server
	m_server.start();
}

void QUaAcFullUi::createWidgetsInDocks()
{
	// instantiate dock manager
	m_dockManager = new QAdDockManager(this);

	// NOTE : it seems layout gets formed as widgets get added, building upon the last state,
	//        the area type (left, right, top, etc.) of the first widget added is irrelevant,
	//        the area of the second widget added defines its location wrt to the first widget,
	//        now the first and second widget form a new area altogether, so the area of the third
	//        widget defines its location wrt to this new area (first + second) and so on.
	//        so as we add widgets we 'pivot' wrt the agroupation of all previous widgets.
	//        to change the 'pivot' we must use the returned area and pass it as the last argument,
	//        then all widgets added with this 'pivot' will be positioned wrt that widget
	int dockMargins = 6;

	auto pDockUsersTable = new QAdDockWidget(tr("Users"), this);
	m_userTable = new QUaUserTable(pDockUsersTable);
	pDockUsersTable->setWidget(m_userTable);
	m_dockManager->addDockWidget(QAd::CenterDockWidgetArea, pDockUsersTable);
	m_userTable->layout()->setContentsMargins(dockMargins, dockMargins, dockMargins, dockMargins);

	auto pDockRolesTable = new QAdDockWidget(tr("Roles"), this);
	m_roleTable = new QUaRoleTable(pDockRolesTable);
	pDockRolesTable->setWidget(m_roleTable);
	m_dockManager->addDockWidget(QAd::RightDockWidgetArea, pDockRolesTable);
	m_roleTable->layout()->setContentsMargins(dockMargins, dockMargins, dockMargins, dockMargins);

	auto pDockPermsTable = new QAdDockWidget(tr("Permissions"), this);
	m_permsTable = new QUaPermissionsTable(pDockPermsTable);
	pDockPermsTable->setWidget(m_permsTable);
	m_dockManager->addDockWidget(QAd::BottomDockWidgetArea, pDockPermsTable);
	m_permsTable->layout()->setContentsMargins(dockMargins, dockMargins, dockMargins, dockMargins);

	auto pDockUserEdit = new QAdDockWidget(tr("User Edit"), this);
	m_userWidget = new QUaUserWidgetEdit(pDockUserEdit);
	pDockUserEdit->setWidget(m_userWidget);
	auto userArea =
	m_dockManager->addDockWidget(QAd::RightDockWidgetArea, pDockUserEdit);
	m_userWidget->layout()->setContentsMargins(dockMargins, dockMargins, dockMargins, dockMargins);

	auto pDockRoleEdit = new QAdDockWidget(tr("Role Edit"), this);
	m_roleWidget = new QUaRoleWidgetEdit(pDockRoleEdit);
	pDockRoleEdit->setWidget(m_roleWidget);
	auto roleArea =
	m_dockManager->addDockWidget(QAd::BottomDockWidgetArea, pDockRoleEdit, userArea);
	m_roleWidget->layout()->setContentsMargins(dockMargins, dockMargins, dockMargins, dockMargins);

	auto pDockPermsEdit = new QAdDockWidget(tr("Permissions Edit"), this);
	m_permsWidget = new QUaPermissionsWidgetEdit(pDockPermsEdit);
	pDockPermsEdit->setWidget(m_permsWidget);
	m_dockManager->addDockWidget(QAd::BottomDockWidgetArea, pDockPermsEdit, roleArea);
	m_permsWidget->layout()->setContentsMargins(dockMargins, dockMargins, dockMargins, dockMargins);
}

void QUaAcFullUi::setupUserWidgets()
{
	Q_CHECK_PTR(m_userTable);
	Q_CHECK_PTR(m_userWidget);
	// disable until some valid object selected
	m_userWidget->setEnabled(false);
	m_userWidget->setRepeatVisible(true);

	// TODO

}

void QUaAcFullUi::setupRoleWidgets()
{
	Q_CHECK_PTR(m_roleTable);
	Q_CHECK_PTR(m_roleWidget);
	// disable until some valid object selected
	m_roleWidget->setEnabled(false);

	// TODO
}

void QUaAcFullUi::setupPermsWidgets()
{
	Q_CHECK_PTR(m_permsTable);
	Q_CHECK_PTR(m_permsWidget);
	// disable until some valid object selected
	m_permsWidget->setEnabled(false);

	// TODO
}
