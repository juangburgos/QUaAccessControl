#include "quaacfullui.h"
#include "ui_quaacfullui.h"

#include <QLayout>
#include <QMessageAuthenticationCode>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>

#include <QUaAccessControl>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>

#include <QUaUserTable>
#include <QUaRoleTable>
#include <QUaPermissionsTable>

#include <QUaAcCommonDialog>
#include <QUaUserWidgetEdit>
#include <QUaRoleWidgetEdit>
#include <QUaPermissionsWidgetEdit>

QUaAcFullUi::QUaAcFullUi(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QUaAcFullUi),
	m_strTitle("%1 - QUaUserAccess GUI")
{
    ui->setupUi(this);
	this->setWindowTitle(m_strTitle.arg(tr("Untitled")));
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
	// setup menu bar
	this->setupMenuBar();
	// handle user changed
	QObject::connect(this, &QUaAcFullUi::loggedUserChanged, this, &QUaAcFullUi::on_loggedUserChanged);
	// intially logged out
	this->logout();
}

QUaAcFullUi::~QUaAcFullUi()
{
	m_deleting = true;
    delete ui;
}

void QUaAcFullUi::on_loggedUserChanged(QUaUser * user)
{
	// get menu bar widgets
	auto logginBar = dynamic_cast<QMenuBar*>(this->menuBar()->cornerWidget());
	Q_CHECK_PTR(logginBar);
	auto menuHelp = logginBar->findChild<QMenu*>("menuHelp");
	Q_CHECK_PTR(menuHelp);
	Q_ASSERT(menuHelp->actions().count() == 1);
	QAction * actLogInOut = menuHelp->actions().at(0);
	// set edit widgets permissions
	this->setWidgetUserEditPermissions(user);
	this->setWidgetRoleEditPermissions(user);
	this->setWidgetPermissionsEditPermissions(user);
	// update ui
	if (!user)
	{
		// update menubar : logged out user
		menuHelp->setTitle(tr("Please Login"));
		actLogInOut->setText(tr("Login"));
		// clear edit widgets
		this->clearWidgetUserEdit();
		this->clearWidgetRoleEdit();
		this->clearWidgetPermissionsEdit();
		return;
	}
	// update menubar : logged in user
	menuHelp->setTitle(user->getName());
	actLogInOut->setText(tr("Logout"));
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
		permissions->remove();
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
		permissions->remove();
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

	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	auto listUsers = ac->users();

	// set ac to table
	QObject::connect(this, &QUaAcFullUi::loggedUserChanged, m_userTable, &QUaUserTable::on_loggedUserChanged);
	m_userTable->setAccessControl(ac);

	// handle table events (user selection)
	// change widgets
	QObject::connect(m_userTable, &QUaUserTable::userSelectionChanged, this,
	[this](QUaUser * userPrev, QUaUser * userCurr)
	{
		Q_UNUSED(userPrev);
		// early exit
		if (!userCurr || m_deleting)
		{
			return;
		}
		// bind widget for current selection
		this->bindWidgetUserEdit(userCurr);
	});

	// setup user edit widget
	m_userWidget->setRoleList(ac->roles());
}

void QUaAcFullUi::setupRoleWidgets()
{
	Q_CHECK_PTR(m_roleTable);
	Q_CHECK_PTR(m_roleWidget);
	// disable until some valid object selected
	m_roleWidget->setEnabled(false);

	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	auto listUsers = ac->users();

	// set ac to table
	QObject::connect(this, &QUaAcFullUi::loggedUserChanged, m_roleTable, &QUaRoleTable::on_loggedUserChanged);
	m_roleTable->setAccessControl(ac);

	// handle table events (user selection)
	// change widgets
	QObject::connect(m_roleTable, &QUaRoleTable::roleSelectionChanged, this,
		[this](QUaRole * rolePrev, QUaRole * roleCurr)
	{
		Q_UNUSED(rolePrev);
		// early exit
		if (!roleCurr || m_deleting)
		{
			return;
		}
		// bind widget for current selection
		this->bindWidgetRoleEdit(roleCurr);
	});
}

void QUaAcFullUi::setupPermsWidgets()
{
	Q_CHECK_PTR(m_permsTable);
	Q_CHECK_PTR(m_permsWidget);
	// disable until some valid object selected
	m_permsWidget->setEnabled(false);

	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	auto listUsers = ac->users();

	// set ac to table
	QObject::connect(this, &QUaAcFullUi::loggedUserChanged, m_permsTable, &QUaPermissionsTable::on_loggedUserChanged);
	m_permsTable->setAccessControl(ac);

	// handle table events (user selection)
	// change widgets
	QObject::connect(m_permsTable, &QUaPermissionsTable::permissionsSelectionChanged, this,
	[this](QUaPermissions * permsPrev, QUaPermissions * permsCurr)
	{
		Q_UNUSED(permsPrev);
		// early exit
		if (!permsCurr || m_deleting)
		{
			return;
		}
		// bind widget for current selection
		this->bindWidgetPermissionsEdit(permsCurr);
	});
}

void QUaAcFullUi::setupMenuBar()
{
	// file open, save, close
	QMenu *menuFile = this->menuBar()->addMenu(tr("File"));
	menuFile->addAction(tr("Open" ), this, &QUaAcFullUi::openFile );
	menuFile->addAction(tr("Save" ), this, &QUaAcFullUi::saveFile );
	menuFile->addAction(tr("Close"), this, &QUaAcFullUi::closeFile);

	// setup top right loggin menu
	auto logginBar  = new QMenuBar(this->menuBar());
	QMenu *menuHelp = new QMenu(tr("Please Login"), logginBar);
	menuHelp->setObjectName("menuHelp");
	logginBar->addMenu(menuHelp);
	this->menuBar()->setCornerWidget(logginBar);
	// add logon/logout action
	QAction * actLogInOut = menuHelp->addAction(tr("Login"));
	// setup action events
	QObject::connect(actLogInOut, &QAction::triggered, this,
	[this]() {
		if (this->loggedUser())
		{
			this->logout();
		}
		else
		{
			this->login();
		}
	});
}

void QUaAcFullUi::openFile()
{
	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	// setup error dialog just in case
	QMessageBox msgBox;
	msgBox.setWindowTitle("Error");
	msgBox.setIcon(QMessageBox::Critical);
	// read from file
	QString strConfigFileName = QFileDialog::getOpenFileName(this, tr("Open File"),
		QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
		tr("XML (*.xml)"));
	// validate
	if (strConfigFileName.isEmpty())
	{
		return;
	}
	// compose key file name
	QString strKeyFileName = QString("%1/%2.key").arg(QFileInfo(strConfigFileName).absoluteDir().absolutePath()).arg(QFileInfo(strConfigFileName).baseName());
	// create files
	QFile fileConfig(strConfigFileName);
	QFile fileKey   (strKeyFileName);
	// exists
	if (!fileConfig.exists())
	{
		msgBox.setText(tr("File %1 does not exist.").arg(strConfigFileName));
		msgBox.exec();
		return;
	}
	else if (!fileKey.exists())
	{
		msgBox.setText(tr("File %1 does not exist.").arg(strKeyFileName));
		msgBox.exec();
		return;
	}
	else if (fileConfig.open(QIODevice::ReadOnly) && fileKey.open(QIODevice::ReadOnly))
	{
		// load config
		auto byteContents = fileConfig.readAll();
		auto byteKey      = fileKey.readAll();
		// compute key
		QByteArray keyComputed = QMessageAuthenticationCode::hash(
			byteContents,
			m_strSecret.toUtf8(),
			QCryptographicHash::Sha512
		).toHex();
		// compare computed key with loaded key
		if (byteKey == keyComputed)
		{
			// close old file
			this->closeFile();
			// try load config
			auto strError = ac->setXmlConfig(byteContents);
			if (strError.contains("Error"))
			{
				msgBox.setText(strError);
				msgBox.exec();
				return;
			}
			// update title
			this->setWindowTitle(m_strTitle.arg(strConfigFileName));
			// login
			this->login();
		}
		else
		{
			msgBox.setText(tr("Corrupted file %1.\nContents do not match key stored in %2.").arg(strConfigFileName).arg(strKeyFileName));
			msgBox.exec();
		}
	}
	else
	{
		msgBox.setText(tr("Files %1, %2 could not be opened.").arg(strConfigFileName).arg(strKeyFileName));
		msgBox.exec();
	}
}

void QUaAcFullUi::saveFile()
{
	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	// select file
	QString strConfigFileName = QFileDialog::getSaveFileName(this, tr("Save File"),
		QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
		tr("XML (*.xml *.txt)"));
	// ignore if empty
	if (strConfigFileName.isEmpty() || strConfigFileName.isNull())
	{
		return;
	}
	// compose key file name
	QString strKeyFileName = QString("%1/%2.key").arg(QFileInfo(strConfigFileName).absoluteDir().absolutePath()).arg(QFileInfo(strConfigFileName).baseName());
	// save to file
	QFile fileConfig(strConfigFileName);
	QFile fileKey(strKeyFileName);
	if (fileConfig.open(QIODevice::ReadWrite | QFile::Truncate) && fileKey.open(QIODevice::ReadWrite | QFile::Truncate))
	{
		QTextStream streamConfig(&fileConfig);
		QTextStream streamKey(&fileKey);
		// create dom doc
		QDomDocument doc;
		// set xml header
		QDomProcessingInstruction header = doc.createProcessingInstruction("xml", "version='1.0' encoding='UTF-8'");
		doc.appendChild(header);
		// convert config to xml
		auto elemAc = ac->toDomElement(doc);
		doc.appendChild(elemAc);
		// get contents bytearray
		auto byteContents = doc.toByteArray();
		// save config in file
		streamConfig << byteContents;
		// create key
		QByteArray key = QMessageAuthenticationCode::hash(
			byteContents,
			m_strSecret.toUtf8(),
			QCryptographicHash::Sha512
		).toHex();
		// save key in file
		streamKey << key;
		// update title
		this->setWindowTitle(m_strTitle.arg(strConfigFileName));
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setWindowTitle("Error");
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.setText(tr("Error opening files %1, %2 for write operations.").arg(strConfigFileName).arg(strKeyFileName));
		msgBox.exec();
	}
	// close files
	fileConfig.close();
	fileKey.close();
}

void QUaAcFullUi::closeFile()
{
	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	// disable old connections
	while (m_connsUserWidget.count() > 0)
	{
		QObject::disconnect(m_connsUserWidget.takeFirst());
	}
	while (m_connsRoleWidget.count() > 0)
	{
		QObject::disconnect(m_connsRoleWidget.takeFirst());
	}
	while (m_connsPermsWidget.count() > 0)
	{
		QObject::disconnect(m_connsPermsWidget.takeFirst());
	}
	// clear config
	// NOTE : need to delete inmediatly, ac->clear(); (deleteLater) won't do the job
	ac->clearInmediatly();
	// clear user edit widget
	this->clearWidgetUserEdit();
	this->clearWidgetRoleEdit();
	this->clearWidgetPermissionsEdit();
	// logout
	this->logout();
	// update title
	this->setWindowTitle(m_strTitle.arg(tr("Untitled")));
}

QUaUser * QUaAcFullUi::loggedUser() const
{
	return m_loggedUser;
}

void QUaAcFullUi::setLoggedUser(QUaUser * user)
{
	m_loggedUser = user;
	emit this->loggedUserChanged(m_loggedUser);
}

void QUaAcFullUi::login()
{
	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	auto listUsers = ac->users();
	// if no users yet, create root user
	if (listUsers->users().count() <= 0)
	{
		// ask user if create new config
		auto res = QMessageBox::question(
			this, 
			tr("No Config Loaded"), 
			tr("There is no configuration loaded.\nWould you like to create a new one?"), 
			QMessageBox::StandardButton::Ok,
			QMessageBox::StandardButton::Cancel
		);
		if (res != QMessageBox::StandardButton::Ok)
		{
			return;
		}
		// setup new user widget
		auto widgetNewUser = new QUaUserWidgetEdit;
		widgetNewUser->setActionsVisible(false);
		widgetNewUser->setRoleVisible(false);
		widgetNewUser->setHashVisible(false);
		widgetNewUser->setRepeatVisible(true);
		// setup dialog
		QUaAcCommonDialog dialog;
		dialog.setWindowTitle(tr("Create Root User"));
		dialog.setWidget(widgetNewUser);
		// show dialog
		this->showCreateRootUserDialog(dialog);
		return;
	}
	// if users existing, setup widget for credentials
	auto widgetNewUser = new QUaUserWidgetEdit;
	widgetNewUser->setActionsVisible(false);
	widgetNewUser->setRoleVisible(false);
	widgetNewUser->setHashVisible(false);
	// setup dialog
	QUaAcCommonDialog dialog;
	dialog.setWindowTitle(tr("Login Credentials"));
	dialog.setWidget(widgetNewUser);
	// show dialog
	this->showUserCredentialsDialog(dialog);
}

void QUaAcFullUi::logout()
{
	// logout
	this->setLoggedUser(nullptr);
}

void QUaAcFullUi::showCreateRootUserDialog(QUaAcCommonDialog & dialog)
{
	int res = dialog.exec();
	if (res != QDialog::Accepted)
	{
		return;
	}
	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	auto listUsers = ac->users();
	// get widget
	auto widgetNewUser = qobject_cast<QUaUserWidgetEdit*>(dialog.widget());
	Q_CHECK_PTR(widgetNewUser);
	// get user data
	QString strUserName = widgetNewUser->userName().trimmed();
	QString strPassword = widgetNewUser->password().trimmed();
	QString strRepeat   = widgetNewUser->repeat().trimmed();
	// check pass and repeat
	if (strPassword.compare(strRepeat, Qt::CaseSensitive) != 0)
	{
		QMessageBox::critical(this, tr("Create Root User Error"), tr("Passwords do not match."), QMessageBox::StandardButton::Ok);
		this->showCreateRootUserDialog(dialog);
		return;
	}
	// check
	QString strError = listUsers->addUser(strUserName, strPassword);
	if (strError.contains("Error"))
	{
		QMessageBox::critical(this, tr("Create Root User Error"), strError, QMessageBox::StandardButton::Ok);
		this->showCreateRootUserDialog(dialog);
		return;
	}
	// else set new root user
	QUaUser * root = listUsers->user(strUserName);
	Q_CHECK_PTR(root);
	ac->setRootUser(root);
	// get root permissions object
	auto rootPermissions = root->permissionsObject();
	if (rootPermissions)
	{
		// only root can add/remove users, roles and permissions
		ac->setPermissionsObject(rootPermissions);
		ac->users()->setPermissionsObject(rootPermissions);
		ac->roles()->setPermissionsObject(rootPermissions);
		ac->permissions()->setPermissionsObject(rootPermissions);
	}
	else
	{
		QObject::connect(root, &QUaUser::permissionsObjectChanged, this,
		[ac](QUaPermissions * rootPermissions) {
			ac->setPermissionsObject(rootPermissions);
			ac->users()->setPermissionsObject(rootPermissions);
			ac->roles()->setPermissionsObject(rootPermissions);
			ac->permissions()->setPermissionsObject(rootPermissions);
		});
	}
	// login root user
	this->setLoggedUser(root);
}

void QUaAcFullUi::showUserCredentialsDialog(QUaAcCommonDialog & dialog)
{
	int res = dialog.exec();
	if (res != QDialog::Accepted)
	{
		if (!this->loggedUser())
		{
			this->logout();
		}
		return;
	}
	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	auto listUsers = ac->users();
	// get widget
	auto widgetNewUser = qobject_cast<QUaUserWidgetEdit*>(dialog.widget());
	Q_CHECK_PTR(widgetNewUser);
	// get user data
	QString strUserName = widgetNewUser->userName();
	QString strPassword = widgetNewUser->password();
	// check user
	QUaUser * user = listUsers->user(strUserName);
	if (!user)
	{
		QMessageBox::critical(this, tr("Login Credentials Error"), tr("Username %1 does not exists.").arg(strUserName), QMessageBox::StandardButton::Ok);
		this->showUserCredentialsDialog(dialog);
		return;
	}
	// check pass
	if (!user->isPasswordValid(strPassword))
	{
		QMessageBox::critical(this, tr("Login Credentials Error"), tr("Invalid password for user %1.").arg(strUserName), QMessageBox::StandardButton::Ok);
		this->showUserCredentialsDialog(dialog);
		return;
	}
	// login
	this->setLoggedUser(user);
}

void QUaAcFullUi::clearWidgetUserEdit()
{
	m_userWidget->setUserName("");
	m_userWidget->setRole(nullptr);
	m_userWidget->setPassword("");
	m_userWidget->setRepeat("");
	m_userWidget->setHash("");
	m_userWidget->setEnabled(false);
}

void QUaAcFullUi::bindWidgetUserEdit(QUaUser * user)
{
	// disable old connections
	while (m_connsUserWidget.count() > 0)
	{
		QObject::disconnect(m_connsUserWidget.takeFirst());
	}
	// bind common
	m_connsUserWidget <<
	QObject::connect(user, &QObject::destroyed, this,
	[this]() {
		if (m_deleting)
		{
			return;
		}
		// clear widget
		this->clearWidgetUserEdit();
	});
	// name
	m_userWidget->setUserNameReadOnly(true);
	m_userWidget->setUserName(user->getName());
	// role
	auto role = user->role();
	m_userWidget->setRole(role);
	m_connsUserWidget <<
	QObject::connect(user, &QUaUser::roleChanged, this,
	[this](QUaRole * role) {
		if (m_deleting)
		{
			return;
		}
		m_userWidget->setRole(role);
	});
	m_connsUserWidget <<
	QObject::connect(role, &QObject::destroyed, user,
	[this, user]() {
		if (m_deleting)
		{
			return;
		}
		m_userWidget->setRole(user->role());
	});
	// hash
	m_userWidget->setHash(user->getHash().toHex());
	m_connsUserWidget <<
	QObject::connect(user, &QUaUser::hashChanged, this,
	[this](const QByteArray &hash) {
		if (m_deleting)
		{
			return;
		}
		m_userWidget->setHash(hash.toHex());
	});

	// password
	m_userWidget->setPassword("");
	m_userWidget->setRepeat("");

	// on click delete
	m_connsUserWidget <<
	QObject::connect(m_userWidget, &QUaUserWidgetEdit::deleteClicked, user,
	[this, user]() {
		// ask for confirmation
		auto res = QMessageBox::warning(
			this, 
			tr("Delete User Confirmation"), 
			tr("Are you sure you want to delete user %1?").arg(user->getName()), 
			QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No
		);
		if (res != QMessageBox::StandardButton::Yes)
		{
			return;
		}
		// delete (emit signal on list)
		user->remove();
	});

	// on click apply
	m_connsUserWidget <<
	QObject::connect(m_userWidget, &QUaUserWidgetEdit::applyClicked, user,
	[this, user]() {
		// udpate role
		user->setRole(m_userWidget->role());
		// update password (if non empty)
		QString strNewPass = m_userWidget->password().trimmed();
		QString strRepeat  = m_userWidget->repeat().trimmed();
		if (strNewPass.isEmpty())
		{
			return;
		}
		// check pass and repeat
		if (strNewPass.compare(strRepeat, Qt::CaseSensitive) != 0)
		{
			QMessageBox::critical(this, tr("Edit User Error"), tr("Passwords do not match."), QMessageBox::StandardButton::Ok);
			return;
		}
		QString strError = user->setPassword(strNewPass);
		if (strError.contains("Error"))
		{
			QMessageBox::critical(this, tr("Edit User Error"), tr("Invalid new password. %1").arg(strError), QMessageBox::StandardButton::Ok);
		}
	});

	// set permissions
	this->setWidgetUserEditPermissions(m_loggedUser);
}

void QUaAcFullUi::setWidgetUserEditPermissions(QUaUser * user)
{
	m_userWidget->setEnabled(true);
	// if no user then clear
	if (!user)
	{
		this->clearWidgetUserEdit();
		return;
	}
	// permission to delete user or modify role come from user list permissions
	auto listPerms = user->list()->permissionsObject();
	if (!listPerms)
	{
		// no perms set, means all permissions
		m_userWidget->setDeleteVisible(true);
		m_userWidget->setRoleReadOnly(false);
	}
	else
	{
		m_userWidget->setDeleteVisible(listPerms->canUserWrite(user));
		m_userWidget->setRoleReadOnly(!listPerms->canUserWrite(user));
	}

	// permission to change password comes from individual user permissions
	auto dispUser = user->list()->user(m_userWidget->userName());
	if (!dispUser)
	{
		this->clearWidgetUserEdit();
		return;
	}
	auto dispPerms = dispUser->permissionsObject();
	// no perms set, means all permissions
	if (!dispPerms)
	{
		m_userWidget->setPasswordVisible(true);
		return;
	}
	if (!dispPerms->canUserRead(user))
	{
		this->clearWidgetUserEdit();
		return;
	}
	if (dispPerms->canUserWrite(user))
	{
		m_userWidget->setPasswordVisible(true);
	}
	else
	{
		m_userWidget->setPasswordVisible(false);
	}
}

void QUaAcFullUi::clearWidgetRoleEdit()
{
	m_roleWidget->setRoleName("");
	m_roleWidget->setUsers(QStringList());
	m_roleWidget->setEnabled(false);
}

void QUaAcFullUi::bindWidgetRoleEdit(QUaRole * role)
{
	// disable old connections
	while (m_connsRoleWidget.count() > 0)
	{
		QObject::disconnect(m_connsRoleWidget.takeFirst());
	}
	// bind common
	m_connsRoleWidget <<
	QObject::connect(role, &QObject::destroyed, this,
	[this]() {
		if (m_deleting)
		{
			return;
		}
		// clear widget
		this->clearWidgetRoleEdit();
	});
	// name
	m_roleWidget->setRoleNameReadOnly(true);
	m_roleWidget->setRoleName(role->getName());
	// users
	QStringList listUsers;
	for (auto user : role->users())
	{
		QString strUserName = user->getName();
		listUsers << strUserName;
		// suscribe used destroyed
		m_connsRoleWidget <<
		QObject::connect(user, &QObject::destroyed, this,
		[this, strUserName]() {
			if (m_deleting)
			{
				return;
			}
			m_roleWidget->removeUser(strUserName);
		});
	}
	m_roleWidget->setUsers(listUsers);
	// subscribe user added
	m_connsRoleWidget <<
	QObject::connect(role, &QUaRole::userAdded, this,
	[this](QUaUser * user) {
		if (m_deleting)
		{
			return;
		}
		QString strUserName = user->getName();
		m_roleWidget->addUser(strUserName);
		// suscribe used destroyed
		m_connsRoleWidget <<
		QObject::connect(user, &QObject::destroyed, this,
		[this, strUserName]() {
			if (m_deleting)
			{
				return;
			}
			m_roleWidget->removeUser(strUserName);
		});
	});
	// subscribe user removed
	m_connsRoleWidget <<
	QObject::connect(role, &QUaRole::userRemoved, this,
	[this](QUaUser * user) {
		if (m_deleting)
		{
			return;
		}
		m_roleWidget->removeUser(user->getName());
	});
	// on click delete
	m_connsRoleWidget <<
	QObject::connect(m_roleWidget, &QUaRoleWidgetEdit::deleteClicked, role,
	[this, role]() {
		// ask for confirmation
		auto res = QMessageBox::warning(
			this,
			tr("Delete Role Confirmation"),
			tr("Are you sure you want to delete role %1?").arg(role->getName()),
			QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No
		);
		if (res != QMessageBox::StandardButton::Yes)
		{
			return;
		}
		// delete (emit signal on list)
		role->remove();
	});

	// set permissions
	this->setWidgetRoleEditPermissions(m_loggedUser);
}

void QUaAcFullUi::setWidgetRoleEditPermissions(QUaUser * user)
{
	m_roleWidget->setEnabled(true);
	// if no user then clear
	if (!user)
	{
		this->clearWidgetRoleEdit();
		return;
	}
	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	// permission to delete role and see users come from role list permissions
	auto dispRole = ac->roles()->role(m_roleWidget->roleName());
	if (!dispRole)
	{
		this->clearWidgetRoleEdit();
		return;
	}
	auto listPerms = dispRole->list()->permissionsObject();
	if (!listPerms)
	{
		// no perms set, means all permissions
		m_roleWidget->setActionsVisible(true);
		m_roleWidget->setUserListVisible(true);
	}
	else
	{
		m_roleWidget->setActionsVisible(listPerms->canUserWrite(user));
		m_roleWidget->setUserListVisible(listPerms->canUserRead(user));
	}
	auto dispPerms = dispRole->permissionsObject();
	// no perms set, means all permissions (only read apply to role, nothing to modify)
	if (!dispPerms)
	{
		return;
	}
	if (!dispPerms->canUserRead(user))
	{
		this->clearWidgetRoleEdit();
	}
}

void QUaAcFullUi::clearWidgetPermissionsEdit()
{
	m_permsWidget->setId("");
	m_permsWidget->setRoleAccessMap(QUaRoleAccessMap());
	m_permsWidget->setUserAccessMap(QUaUserAccessMap());
	m_permsWidget->setEnabled(false);
}

void QUaAcFullUi::bindWidgetPermissionsEdit(QUaPermissions * perms)
{
	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	// disable old connections
	while (m_connsPermsWidget.count() > 0)
	{
		QObject::disconnect(m_connsPermsWidget.takeFirst());
	}
	// bind common
	m_connsPermsWidget <<
	QObject::connect(perms, &QObject::destroyed, this,
	[this]() {
		if (m_deleting)
		{
			return;
		}
		// clear widget
		this->clearWidgetPermissionsEdit();
	});
	// name
	m_permsWidget->setIdReadOnly(true);
	m_permsWidget->setId(perms->getId());

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
	m_permsWidget->setRoleAccessMap(mapRoles);

	// users (all)
	QUaUserAccessMap mapUsers;
	auto users = ac->users()->users();
	for (auto user : users)
	{
		mapUsers[user->getName()] = {
			perms->canUserReadDirectly (user),
			perms->canUserWriteDirectly(user),
			perms->canRoleRead (user->role()),
			perms->canRoleWrite(user->role())
		};
	}
	m_permsWidget->setUserAccessMap(mapUsers);

	// role updates
	auto updateRole = [this, perms](QUaRole * role) {
		// role table
		m_permsWidget->updateRoleAccess(
			role->getName(),
			{ perms->canRoleRead(role), perms->canRoleWrite(role) }
		);
		// user table
		auto users = role->users();
		for (auto user : users)
		{
			m_permsWidget->updateUserAccess(
				user->getName(), 
				{
				perms->canUserReadDirectly (user),
				perms->canUserWriteDirectly(user),
				perms->canRoleRead (user->role()),
				perms->canRoleWrite(user->role())
				}
			);
		}
	};
	m_connsPermsWidget << QObject::connect(perms, &QUaPermissions::canReadRoleAdded   , this, updateRole, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(perms, &QUaPermissions::canReadRoleRemoved , this, updateRole, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(perms, &QUaPermissions::canWriteRoleAdded  , this, updateRole, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(perms, &QUaPermissions::canWriteRoleRemoved, this, updateRole, Qt::QueuedConnection);

	// user updates
	auto updateUser = [this, perms](QUaUser * user) {
		m_permsWidget->updateUserAccess(
			user->getName(), 
			{
			perms->canUserReadDirectly (user),
			perms->canUserWriteDirectly(user),
			perms->canRoleRead (user->role()),
			perms->canRoleWrite(user->role())
			}
		);
	};
	m_connsPermsWidget << QObject::connect(perms, &QUaPermissions::canReadUserAdded   , this, updateUser, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(perms, &QUaPermissions::canReadUserRemoved , this, updateUser, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(perms, &QUaPermissions::canWriteUserAdded  , this, updateUser, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(perms, &QUaPermissions::canWriteUserRemoved, this, updateUser, Qt::QueuedConnection);

	// user changes role
	for (auto user : users)
	{
		m_connsPermsWidget << QObject::connect(user, &QUaUser::roleChanged, this, 
		[updateUser, user]() {
			updateUser(user);
		}, Qt::QueuedConnection);
	}

	// on user or role added/removed
	auto resetPermsWidget = [this, perms]() {
		this->bindWidgetPermissionsEdit(perms);
	};
	// NOTE : queued to wait until user/role has name or has actually been deleted
	m_connsPermsWidget << QObject::connect(ac->roles(), &QUaRoleList::roleAdded  , perms, resetPermsWidget, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(ac->roles(), &QUaRoleList::roleRemoved, perms, resetPermsWidget, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(ac->users(), &QUaUserList::userAdded  , perms, resetPermsWidget, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(ac->users(), &QUaUserList::userRemoved, perms, resetPermsWidget, Qt::QueuedConnection);

	// on click apply
	m_connsPermsWidget <<
	QObject::connect(m_permsWidget, &QUaPermissionsWidgetEdit::applyClicked, perms,
	[this, ac, perms]() {
		// update roles access
		auto roleMap = m_permsWidget->roleAccessMap();
		QUaRoleAccessIter i(roleMap);
		while (i.hasNext())
		{
			i.next();		
			auto role = ac->roles()->role(i.key());
			Q_CHECK_PTR(role);
			i.value().canRead  ? perms->addRoleCanRead (role) : perms->removeRoleCanRead (role);
			i.value().canWrite ? perms->addRoleCanWrite(role) : perms->removeRoleCanWrite(role);
		}
		// update users access
		auto userMap = m_permsWidget->userAccessMap();
		QUaUserAccessIter k(userMap);
		while (k.hasNext())
		{
			k.next();
			auto user = ac->users()->user(k.key());
			Q_CHECK_PTR(user);
			k.value().canUserRead  ? perms->addUserCanRead (user) : perms->removeUserCanRead (user);
			k.value().canUserWrite ? perms->addUserCanWrite(user) : perms->removeUserCanWrite(user);
		}
	});

	// on click delete
	m_connsPermsWidget <<
	QObject::connect(m_permsWidget, &QUaPermissionsWidgetEdit::deleteClicked, perms,
	[this, perms]() {
		// ask for confirmation
		auto res = QMessageBox::warning(
			this,
			tr("Delete Permissions Confirmation"),
			tr("Are you sure you want to delete permissions object %1?").arg(perms->getId()),
			QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No
		);
		if (res != QMessageBox::StandardButton::Yes)
		{
			return;
		}
		// delete (emit signal on list)
		perms->remove();
	});

	// set permissions
	this->setWidgetPermissionsEditPermissions(m_loggedUser);
}

void QUaAcFullUi::setWidgetPermissionsEditPermissions(QUaUser * user)
{
	m_permsWidget->setEnabled(true);
	// if no user then clear
	if (!user)
	{
		this->clearWidgetPermissionsEdit();
		return;
	}
	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	// permission to delete permissions and see role/user access come from role list permissions
	auto dispPerms = ac->permissions()->permission(m_permsWidget->id());
	if (!dispPerms)
	{
		this->clearWidgetPermissionsEdit();
		return;
	}
	auto listPerms = dispPerms->list()->permissionsObject();
	if (!listPerms)
	{
		// no perms set, means all permissions
		m_permsWidget->setActionsVisible(true);
		m_permsWidget->setAccessVisible(true);
	}
	else
	{
		m_permsWidget->setActionsVisible(listPerms->canUserWrite(user));
		m_permsWidget->setAccessVisible(listPerms->canUserRead(user));
	}
	auto dispPermsPerms = dispPerms->permissionsObject();
	// no perms set, means all permissions (only read apply to perms, nothing else left to modify)
	if (!dispPermsPerms)
	{
		return;
	}
	if (!dispPermsPerms->canUserRead(user))
	{
		this->clearWidgetPermissionsEdit();
	}
}
