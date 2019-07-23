#include "quapermissionstabletestdialog.h"
#include "ui_quapermissionstabletestdialog.h"

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

#include <QUaAcCommonDialog>
#include <QUaPermissionsWidgetEdit>
#include <QUaUserWidgetEdit>

QUaPermissionsTableTestDialog::QUaPermissionsTableTestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QUaPermissionsTableTestDialog)
{
    ui->setupUi(this);
	m_deleting = false;
	// disable until some valid object selected
	ui->widgetPermissionsEdit->setEnabled(false);
	// logged in user
	m_loggedUser = nullptr;
	QObject::connect(this, &QUaPermissionsTableTestDialog::loggedUserChanged, this, &QUaPermissionsTableTestDialog::on_loggedUserChanged);
	// app secret
	m_strSecret = "my_secret";
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

	// set ac to table
	QObject::connect(this, &QUaPermissionsTableTestDialog::loggedUserChanged, ui->widgetPermissionsTable, &QUaPermissionsTable::on_loggedUserChanged);
	ui->widgetPermissionsTable->setAccessControl(ac);

	// handle table events (user selection)
	// change widgets
	QObject::connect(ui->widgetPermissionsTable, &QUaPermissionsTable::permissionsSelectionChanged, this,
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

	// start server
	m_server.start();

	// intially logged out
	this->logout();
}

QUaPermissionsTableTestDialog::~QUaPermissionsTableTestDialog()
{
	m_deleting = true;
    delete ui;
}

void QUaPermissionsTableTestDialog::on_pushButtonImport_clicked()
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
			// clear old config
			this->clearApplication();
			// try load config
			auto strError = ac->setXmlConfig(byteContents);
			if (strError.contains("Error"))
			{
				msgBox.setText(strError);
				msgBox.exec();
				return;
			}
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

void QUaPermissionsTableTestDialog::on_pushButtonExport_clicked()
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

void QUaPermissionsTableTestDialog::on_pushButtonLogin_clicked()
{
	this->login();
}

void QUaPermissionsTableTestDialog::on_pushButtonLogout_clicked()
{
	this->logout();
}

void QUaPermissionsTableTestDialog::on_loggedUserChanged(QUaUser * user)
{
	// set user edit widget permissions
	this->setWidgetPermissionsEditPermissions(user);
	// update ui
	if (!user)
	{
		// logged out user
		ui->lineEditLoggedUser->setText("");
		ui->pushButtonLogout->setEnabled(false);
		// clear user edit widget
		this->clearWidgetPermissionsEdit();
		return;
	}
	// logged in user
	ui->lineEditLoggedUser->setText(user->getName());
	ui->pushButtonLogout->setEnabled(true);
}

QUaUser * QUaPermissionsTableTestDialog::loggedUser() const
{
	return m_loggedUser;
}

void QUaPermissionsTableTestDialog::setLoggedUser(QUaUser * user)
{
	m_loggedUser = user;
	emit this->loggedUserChanged(m_loggedUser);
}

void QUaPermissionsTableTestDialog::login()
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

void QUaPermissionsTableTestDialog::logout()
{
	// logout
	this->setLoggedUser(nullptr);
}

void QUaPermissionsTableTestDialog::showCreateRootUserDialog(QUaAcCommonDialog & dialog)
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

void QUaPermissionsTableTestDialog::showUserCredentialsDialog(QUaAcCommonDialog & dialog)
{
	int res = dialog.exec();
	if (res != QDialog::Accepted)
	{
		if (!this->loggedUser())
		{
			// logged out user
			ui->lineEditLoggedUser->setText("");
			ui->pushButtonLogout->setEnabled(false);
			// clear user edit widget
			this->clearWidgetPermissionsEdit();
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

void QUaPermissionsTableTestDialog::clearApplication()
{
	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	// disable old connections
	while (m_connections.count() > 0)
	{
		QObject::disconnect(m_connections.takeFirst());
	}
	// clear config
	ac->clear();
	// clear user edit widget
	this->clearWidgetPermissionsEdit();
}

void QUaPermissionsTableTestDialog::clearWidgetPermissionsEdit()
{
	ui->widgetPermissionsEdit->setId("");
	ui->widgetPermissionsEdit->setRoleAccessMap(QUaRoleAccessMap());
	ui->widgetPermissionsEdit->setUserAccessMap(QUaUserAccessMap());
	ui->widgetPermissionsEdit->setEnabled(false);
}

void QUaPermissionsTableTestDialog::bindWidgetPermissionsEdit(QUaPermissions * perms)
{
	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
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
	// name
	ui->widgetPermissionsEdit->setIdReadOnly(true);
	ui->widgetPermissionsEdit->setId(perms->getId());

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
	ui->widgetPermissionsEdit->setRoleAccessMap(mapRoles);

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
	ui->widgetPermissionsEdit->setUserAccessMap(mapUsers);

	// role updates
	auto updateRole = [this, perms](QUaRole * role) {
		// role table
		ui->widgetPermissionsEdit->updateRoleAccess(
			role->getName(),
			{ perms->canRoleRead(role), perms->canRoleWrite(role) }
		);
		// user table
		auto users = role->users();
		for (auto user : users)
		{
			ui->widgetPermissionsEdit->updateUserAccess(
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
	m_connections << QObject::connect(perms, &QUaPermissions::canReadRoleAdded   , this, updateRole, Qt::QueuedConnection);
	m_connections << QObject::connect(perms, &QUaPermissions::canReadRoleRemoved , this, updateRole, Qt::QueuedConnection);
	m_connections << QObject::connect(perms, &QUaPermissions::canWriteRoleAdded  , this, updateRole, Qt::QueuedConnection);
	m_connections << QObject::connect(perms, &QUaPermissions::canWriteRoleRemoved, this, updateRole, Qt::QueuedConnection);

	// user updates
	auto updateUser = [this, perms](QUaUser * user) {
		ui->widgetPermissionsEdit->updateUserAccess(
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

	// on click apply
	m_connections <<
	QObject::connect(ui->widgetPermissionsEdit, &QUaPermissionsWidgetEdit::applyClicked, perms,
	[this, ac, perms]() {
		// update roles access
		auto roleMap = ui->widgetPermissionsEdit->roleAccessMap();
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
		auto userMap = ui->widgetPermissionsEdit->userAccessMap();
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
	m_connections <<
	QObject::connect(ui->widgetPermissionsEdit, &QUaPermissionsWidgetEdit::deleteClicked, perms,
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
		// delete
		perms->remove();
	});

	// set permissions
	this->setWidgetPermissionsEditPermissions(m_loggedUser);
}

void QUaPermissionsTableTestDialog::setWidgetPermissionsEditPermissions(QUaUser * user)
{
	ui->widgetPermissionsEdit->setEnabled(true);
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
	auto dispPerms = ac->permissions()->permission(ui->widgetPermissionsEdit->id());
	if (!dispPerms)
	{
		this->clearWidgetPermissionsEdit();
		return;
	}
	auto listPerms = dispPerms->list()->permissionsObject();
	if (!listPerms)
	{
		// no perms set, means all permissions
		ui->widgetPermissionsEdit->setActionsVisible(true);
		ui->widgetPermissionsEdit->setAccessVisible(true);
	}
	else
	{
		ui->widgetPermissionsEdit->setActionsVisible(listPerms->canUserWrite(user));
		ui->widgetPermissionsEdit->setAccessVisible(listPerms->canUserRead(user));
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
