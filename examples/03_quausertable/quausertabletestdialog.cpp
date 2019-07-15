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
		QString strPermId = QString("only_%1").arg(user->getName());
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

	// TODO : set users to table

	// TODO : handle table events (user selection)

	// start server
	m_server.start();
}

QUaUserTableTestDialog::~QUaUserTableTestDialog()
{
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
	QString strKeyFileName = QString("%1.key").arg(QFileInfo(strConfigFileName).baseName());
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
	QString strKeyFileName = QString("%1.key").arg(QFileInfo(strConfigFileName).baseName());
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

void QUaUserTableTestDialog::on_pushButtonApply_clicked()
{
	// TODO : 
}

void QUaUserTableTestDialog::on_loggedUserChanged(QUaUser * user)
{
	if (!user)
	{
		ui->lineEditLoggedUser->setText("");
		return;
	}
	ui->lineEditLoggedUser->setText(user->getName());
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
		// setup new user widget
		auto widgetNewUser = new QUaUserWidgetEdit;
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
	widgetNewUser->setRoleVisible(false);
	widgetNewUser->setHashVisible(false);
	// setup dialog
	QUaAcCommonDialog dialog;
	dialog.setWindowTitle(tr("Login Credentials"));
	dialog.setWidget(widgetNewUser);
	// show dialog
	this->showUserCredentialsDialog(dialog);
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
	QString strUserName = widgetNewUser->userName();
	QString strPassword = widgetNewUser->password();
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
		ac->setPermissionsObject(rootPermissions);
	}
	else
	{
		QObject::connect(root, &QUaUser::permissionsObjectChanged, this,
		[ac](QUaPermissions * rootPermissions) {
			ac->setPermissionsObject(rootPermissions);
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

void QUaUserTableTestDialog::on_pushButtonLogout_clicked()
{
	// logout
	this->setLoggedUser(nullptr);
}