#include "quaroletabletestdialog.h"
#include "ui_quaroletabletestdialog.h"

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
#include <QUaRoleWidgetEdit>
#include <QUaUserWidgetEdit>

QUaRoleTableTestDialog::QUaRoleTableTestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QUaRoleTableTestDialog)
{
    ui->setupUi(this);
	m_deleting = false;
	// hide apply button until some valid object selected
	ui->widgetRoleEdit->setEnabled(false);
	// logged in user
	m_loggedUser = nullptr;
	QObject::connect(this, &QUaRoleTableTestDialog::loggedUserChanged, this, &QUaRoleTableTestDialog::on_loggedUserChanged);
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

	// set ac to table
	QObject::connect(this, &QUaRoleTableTestDialog::loggedUserChanged, ui->widgetRoleTable, &QUaRoleTable::on_loggedUserChanged);
	ui->widgetRoleTable->setAccessControl(ac);

	// handle table events (user selection)
	// change widgets
	QObject::connect(ui->widgetRoleTable, &QUaRoleTable::roleSelectionChanged, this,
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

	// start server
	m_server.start();

	// intially logged out
	this->logout();
}

QUaRoleTableTestDialog::~QUaRoleTableTestDialog()
{
	m_deleting = true;
    delete ui;
}

void QUaRoleTableTestDialog::on_pushButtonImport_clicked()
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
			msgBox.setText(tr("Corrupted file %1. Contents does not match key stored in %2.").arg(strConfigFileName).arg(strKeyFileName));
			msgBox.exec();
		}
	}
	else
	{
		msgBox.setText(tr("Files %1, %2 could not be opened.").arg(strConfigFileName).arg(strKeyFileName));
		msgBox.exec();
	}
}

void QUaRoleTableTestDialog::on_pushButtonExport_clicked()
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

void QUaRoleTableTestDialog::on_pushButtonLogin_clicked()
{
	this->login();
}

void QUaRoleTableTestDialog::on_pushButtonLogout_clicked()
{
	this->logout();
}

void QUaRoleTableTestDialog::on_loggedUserChanged(QUaUser * user)
{
	// set user edit widget permissions
	this->setWidgetRoleEditPermissions(user);
	// update ui
	if (!user)
	{
		// logged out user
		ui->lineEditLoggedUser->setText("");
		ui->pushButtonLogout->setEnabled(false);
		// clear user edit widget
		this->clearWidgetRoleEdit();
		return;
	}
	// logged in user
	ui->lineEditLoggedUser->setText(user->getName());
	ui->pushButtonLogout->setEnabled(true);
}

QUaUser * QUaRoleTableTestDialog::loggedUser() const
{
	return m_loggedUser;
}

void QUaRoleTableTestDialog::setLoggedUser(QUaUser * user)
{
	m_loggedUser = user;
	emit this->loggedUserChanged(m_loggedUser);
}

void QUaRoleTableTestDialog::login()
{
	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	auto listUsers = ac->users();
	// if no users yet, create root user
	if (listUsers->users().count() <= 0)
	{
		// setup new user widget
		auto widgetNewUser = new QUaUserWidgetEdit;
		widgetNewUser->setActionsVisible(false);
		widgetNewUser->setRoleVisible(false);
		widgetNewUser->setHashVisible(false);
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

void QUaRoleTableTestDialog::logout()
{
	// logout
	this->setLoggedUser(nullptr);
}

void QUaRoleTableTestDialog::showCreateRootUserDialog(QUaAcCommonDialog & dialog)
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

void QUaRoleTableTestDialog::showUserCredentialsDialog(QUaAcCommonDialog & dialog)
{
	int res = dialog.exec();
	if (res != QDialog::Accepted)
	{
		if (!this->loggedUser())
		{
			this->clearApplication();
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

void QUaRoleTableTestDialog::clearApplication()
{
	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	// clear config
	ac->clear();
	// clear user edit widget
	this->clearWidgetRoleEdit();
}

void QUaRoleTableTestDialog::clearWidgetRoleEdit()
{
	ui->widgetRoleEdit->setRoleName("");
	ui->widgetRoleEdit->setEnabled(false);
	ui->widgetRoleEdit->setUsers(QStringList());
}

void QUaRoleTableTestDialog::bindWidgetRoleEdit(QUaRole * role)
{
	// disable old connections
	while (m_connections.count() > 0)
	{
		QObject::disconnect(m_connections.takeFirst());
	}
	// bind common
	m_connections <<
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
	ui->widgetRoleEdit->setRoleNameReadOnly(true);
	ui->widgetRoleEdit->setRoleName(role->getName());
	// users
	QStringList listUsers;
	for (auto user : role->users())
	{
		QString strUserName = user->getName();
		listUsers << strUserName;
		// suscribe used destroyed
		m_connections <<
		QObject::connect(user, &QObject::destroyed, this,
		[this, strUserName]() {
			if (m_deleting)
			{
				return;
			}
			ui->widgetRoleEdit->removeUser(strUserName);
		});
	}
	ui->widgetRoleEdit->setUsers(listUsers);
	// subscribe user added
	m_connections <<
	QObject::connect(role, &QUaRole::userAdded, this,
	[this](QUaUser * user) {
		if (m_deleting)
		{
			return;
		}
		ui->widgetRoleEdit->addUser(user->getName());
	});
	// subscribe user removed
	m_connections <<
	QObject::connect(role, &QUaRole::userRemoved, this,
	[this](QUaUser * user) {
		if (m_deleting)
		{
			return;
		}
		ui->widgetRoleEdit->removeUser(user->getName());
	});
	// on click delete
	m_connections <<
	QObject::connect(ui->widgetRoleEdit, &QUaRoleWidgetEdit::deleteClicked, role,
	[role]() {
		role->deleteLater();
	});

	// set permissions
	this->setWidgetRoleEditPermissions(m_loggedUser);
}

void QUaRoleTableTestDialog::setWidgetRoleEditPermissions(QUaUser * user)
{
	ui->widgetRoleEdit->setEnabled(true);
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
	auto dispRole  = ac->roles()->role(ui->widgetRoleEdit->roleName());
	if (!dispRole)
	{
		this->clearWidgetRoleEdit();
		return;
	}
	auto listPerms = dispRole->list()->permissionsObject();
	if (!listPerms)
	{
		// no perms set, means all permissions
		ui->widgetRoleEdit->setActionsVisible(true);
		ui->widgetRoleEdit->setUserListVisible(true);
	}
	else
	{
		ui->widgetRoleEdit->setActionsVisible(listPerms->canUserWrite(user));
		ui->widgetRoleEdit->setUserListVisible(listPerms->canUserRead(user));
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
