#include "quausertabletestdialog.h"
#include "ui_quausertabletestdialog.h"

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
#include <QUaUserWidgetEdit>

QUaUserTableTestDialog::QUaUserTableTestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QUaUserTableTestDialog)
{
    ui->setupUi(this);
	m_deleting = false;
	// disable until some valid object selected
	ui->widgetUserEdit->setEnabled(false);
	ui->widgetUserEdit->setRepeatVisible(true);
	// logged in user
	m_loggedUser = nullptr;
	QObject::connect(this, &QUaUserTableTestDialog::loggedUserChanged, this, &QUaUserTableTestDialog::on_loggedUserChanged);
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
		permissions->deleteLater();
	});

	// set ac to table
	QObject::connect(this, &QUaUserTableTestDialog::loggedUserChanged, ui->widgetUserTable, &QUaUserTable::on_loggedUserChanged);
	ui->widgetUserTable->setAccessControl(ac);

	// handle table events (user selection)
	// change widgets
	QObject::connect(ui->widgetUserTable, &QUaUserTable::userSelectionChanged, this,
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

	// setup edit widget
	ui->widgetUserEdit->setRoleList(ac->roles());

	// start server
	m_server.start();

	// intially logged out
	this->logout();
}

QUaUserTableTestDialog::~QUaUserTableTestDialog()
{
	m_deleting = true;
    delete ui;
}

void QUaUserTableTestDialog::on_pushButtonImport_clicked()
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

void QUaUserTableTestDialog::on_pushButtonExport_clicked()
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

void QUaUserTableTestDialog::on_pushButtonLogin_clicked()
{
	this->login();
}

void QUaUserTableTestDialog::on_pushButtonLogout_clicked()
{
	this->logout();
}

void QUaUserTableTestDialog::on_loggedUserChanged(QUaUser * user)
{
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
}

QUaUser * QUaUserTableTestDialog::loggedUser() const
{
	return m_loggedUser;
}

void QUaUserTableTestDialog::setLoggedUser(QUaUser * user)
{
	m_loggedUser = user;
	emit this->loggedUserChanged(m_loggedUser);
}

void QUaUserTableTestDialog::login()
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
		QUaAcCommonDialog dialog(this);
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
	QUaAcCommonDialog dialog(this);
	dialog.setWindowTitle(tr("Login Credentials"));
	dialog.setWidget(widgetNewUser);
	// show dialog
	this->showUserCredentialsDialog(dialog);
}

void QUaUserTableTestDialog::logout()
{
	// logout
	this->setLoggedUser(nullptr);
}

void QUaUserTableTestDialog::showCreateRootUserDialog(QUaAcCommonDialog & dialog)
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

void QUaUserTableTestDialog::showUserCredentialsDialog(QUaAcCommonDialog & dialog)
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
			this->clearWidgetUserEdit();
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

void QUaUserTableTestDialog::clearApplication()
{
	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	// clear config
	ac->clear();
	// clear user edit widget
	this->clearWidgetUserEdit();
}

void QUaUserTableTestDialog::clearWidgetUserEdit()
{
	ui->widgetUserEdit->setUserName("");
	ui->widgetUserEdit->setRole(nullptr);
	ui->widgetUserEdit->setPassword("");
	ui->widgetUserEdit->setRepeat("");
	ui->widgetUserEdit->setHash("");
	ui->widgetUserEdit->setEnabled(false);
}

void QUaUserTableTestDialog::bindWidgetUserEdit(QUaUser * user)
{
	// disable old connections
	while (m_connections.count() > 0)
	{
		QObject::disconnect(m_connections.takeFirst());
	}
	// bind common
	m_connections <<
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
	ui->widgetUserEdit->setUserNameReadOnly(true);
	ui->widgetUserEdit->setUserName(user->getName());
	// role
	auto role = user->role();
	ui->widgetUserEdit->setRole(role);
	m_connections <<
	QObject::connect(user, &QUaUser::roleChanged, this,
	[this](QUaRole * role) {
		if (m_deleting)
		{
			return;
		}
		ui->widgetUserEdit->setRole(role);
	});
	m_connections <<
	QObject::connect(role, &QObject::destroyed, user,
	[this, user]() {
		if (m_deleting)
		{
			return;
		}
		ui->widgetUserEdit->setRole(user->role());
	});
	// hash
	ui->widgetUserEdit->setHash(user->getHash().toHex());
	m_connections <<
	QObject::connect(user, &QUaUser::hashChanged, this,
	[this](const QByteArray &hash) {
		if (m_deleting)
		{
			return;
		}
		ui->widgetUserEdit->setHash(hash.toHex());
	});

	// password
	ui->widgetUserEdit->setPassword("");
	ui->widgetUserEdit->setRepeat("");

	// on click delete
	m_connections <<
	QObject::connect(ui->widgetUserEdit, &QUaUserWidgetEdit::deleteClicked, user,
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
		// delete
		user->remove();
	});

	// on click apply
	m_connections <<
	QObject::connect(ui->widgetUserEdit, &QUaUserWidgetEdit::applyClicked, user,
	[this, user]() {
		// udpate role
		user->setRole(ui->widgetUserEdit->role());
		// update password (if non empty)
		QString strNewPass = ui->widgetUserEdit->password().trimmed();
		QString strRepeat  = ui->widgetUserEdit->repeat().trimmed();
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

void QUaUserTableTestDialog::setWidgetUserEditPermissions(QUaUser * user)
{
	ui->widgetUserEdit->setEnabled(true);
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
		ui->widgetUserEdit->setDeleteVisible(true);
		ui->widgetUserEdit->setRoleReadOnly(false);
	}
	else
	{
		ui->widgetUserEdit->setDeleteVisible(listPerms->canUserWrite(user));
		ui->widgetUserEdit->setRoleReadOnly(!listPerms->canUserWrite(user));
	}

	// permission to change password comes from individual user permissions
	auto dispUser = user->list()->user(ui->widgetUserEdit->userName());
	if (!dispUser)
	{
		this->clearWidgetUserEdit();
		return;
	}
	auto dispPerms = dispUser->permissionsObject();
	// no perms set, means all permissions
	if (!dispPerms)
	{
		ui->widgetUserEdit->setPasswordVisible(true);
		return;
	}
	if (!dispPerms->canUserRead(user))
	{
		this->clearWidgetUserEdit();
		return;
	}
	if (dispPerms->canUserWrite(user))
	{
		ui->widgetUserEdit->setPasswordVisible(true);
	}
	else
	{
		ui->widgetUserEdit->setPasswordVisible(false);
	}
}
