#include <QCoreApplication>
#include <QMessageAuthenticationCode>
#include <QFile>
#include <QDebug>

#include <QUaServer>
#include <QUaAccessControl>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>

#include <FunctionUtils>

QString strSecret         = "my_secret";
QString strConfigFileName = "AcConfig.xml";
QString strKeyFileName    = "AcConfig.key";

bool configLoad(QUaAccessControl * ac)
{
	bool success = false;
	QFile fileConfig(strConfigFileName);
	QFile fileKey   (strKeyFileName);
	// check exists and can be opened
	if (!fileConfig.exists())
	{
		qWarning() << QObject::tr("File %1 does not exist.").arg(strConfigFileName);
		// having no config is acceptable for the first time
		success = true;
	}
	else if (!fileKey.exists())
	{
		qWarning() << QObject::tr("File %1 does not exist.").arg(strKeyFileName);
	}
	else if (fileConfig.open(QIODevice::ReadOnly) && fileKey.open(QIODevice::ReadOnly))
	{
		auto byteContents = fileConfig.readAll();
		auto byteKey      = fileKey.readAll();
		// compute key
		QByteArray keyComputed = QMessageAuthenticationCode::hash(
			byteContents,
			strSecret.toUtf8(),
			QCryptographicHash::Sha512
		).toHex();
		// compare computed key with loaded key
		if (byteKey == keyComputed)
		{
			// try load config
			auto strError = ac->setXmlConfig(byteContents);
			if (strError.contains("Error"))
			{
				qWarning() << strError;
			}
			else
			{
				// config loaded correctly
				success = true;
			}
		}
		else
		{
			qWarning() << QObject::tr("Corrupted file %1. Contents does not match key stored in %2.").arg(strConfigFileName).arg(strKeyFileName);
		}
	}
	else
	{
		qWarning() << QObject::tr("File %1, %2 could not be opened.").arg(strConfigFileName).arg(strKeyFileName);
	}
	// close files
	fileConfig.close();
	fileKey.close();
	// return result
	return success;
}

void configSave(QUaAccessControl * ac)
{
	// save to file
	QString strSaveError;
	QFile fileConfig(strConfigFileName);
	QFile fileKey   (strKeyFileName);
	if (fileConfig.open(QIODevice::ReadWrite | QFile::Truncate) && fileKey.open(QIODevice::ReadWrite | QFile::Truncate))
	{
		QTextStream streamConfig(&fileConfig);
		QTextStream streamKey   (&fileKey);
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
			strSecret.toUtf8(),
			QCryptographicHash::Sha512
		).toHex();
		// save key in file
		streamKey << key;
	}
	else
	{
		qCritical() << QObject::tr("Error opening files %1, %2 for write operations.").arg(strConfigFileName).arg(strKeyFileName);
	}
	// close files
	fileConfig.close();
	fileKey.close();
}

void autoConfigSave(QUaAccessControl * ac)
{
	// save inmediatly on any change, then wait 5 secs for any further changes and if any, save again after wait
	auto throttledConfigSave = std::bind(FunctionUtils::Throttle(&configSave, 2000), ac);
	// changes in ac
	QObject::connect(ac, &QUaAccessControl::permissionsObjectChanged, throttledConfigSave);
	// changes in users list
	QObject::connect(ac->users(), &QUaUserList::permissionsObjectChanged, throttledConfigSave);
	QObject::connect(ac->users(), &QUaUserList::userAdded,
	[throttledConfigSave](QUaUser * user) {
		throttledConfigSave();
		// changes in user
		QObject::connect(user, &QUaUser::permissionsObjectChanged, throttledConfigSave);
		QObject::connect(user, &QUaUser::hashChanged, throttledConfigSave);
		QObject::connect(user, &QUaUser::roleChanged, throttledConfigSave);
	});
	QObject::connect(ac->users(), &QUaUserList::userRemoved, ac->users(), throttledConfigSave, Qt::QueuedConnection);
	// changes in roles list
	QObject::connect(ac->roles(), &QUaRoleList::permissionsObjectChanged, throttledConfigSave);
	QObject::connect(ac->roles(), &QUaRoleList::roleAdded  , 
	[throttledConfigSave](QUaRole * role) {
		// changes in role
		QObject::connect(role, &QUaRole::permissionsObjectChanged, throttledConfigSave);
	});
	QObject::connect(ac->roles(), &QUaRoleList::roleRemoved, ac->roles(), throttledConfigSave, Qt::QueuedConnection);
	// changes in permissions list
	QObject::connect(ac->permissions(), &QUaPermissionsList::permissionsObjectChanged, throttledConfigSave);
	QObject::connect(ac->permissions(), &QUaPermissionsList::permissionsAdded,
	[throttledConfigSave](QUaPermissions * permissions) {
		throttledConfigSave();
		// changes in user
		QObject::connect(permissions, &QUaPermissions::permissionsObjectChanged, throttledConfigSave);
		QObject::connect(permissions, &QUaPermissions::canReadRoleAdded   , throttledConfigSave);
		QObject::connect(permissions, &QUaPermissions::canReadRoleRemoved , throttledConfigSave);
		QObject::connect(permissions, &QUaPermissions::canWriteRoleAdded  , throttledConfigSave);
		QObject::connect(permissions, &QUaPermissions::canWriteRoleRemoved, throttledConfigSave);
		QObject::connect(permissions, &QUaPermissions::canReadUserAdded   , throttledConfigSave);
		QObject::connect(permissions, &QUaPermissions::canReadUserRemoved , throttledConfigSave);
		QObject::connect(permissions, &QUaPermissions::canWriteUserAdded  , throttledConfigSave);
		QObject::connect(permissions, &QUaPermissions::canWriteUserRemoved, throttledConfigSave);
	});
	QObject::connect(ac->permissions(), &QUaPermissionsList::permissionsRemoved, ac->permissions(), throttledConfigSave, Qt::QueuedConnection);
}

void autoUserPermissions(QUaAccessControl * ac)
{
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
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	QUaFolderObject * objsFolder = server.objectsFolder();

	// setup access control information model
	auto accessControl = objsFolder->addChild<QUaAccessControl>();
	accessControl->setDisplayName("AccessControl");
	accessControl->setBrowseName ("AccessControl");

	// load existing config if any
	if (!configLoad(accessControl))
	{
		return -1;
	}

	// setup auto config save
	autoConfigSave(accessControl);

	// setup auto create user permissions
	autoUserPermissions(accessControl);

	auto listUsers = accessControl->users();

	// if no users, then create default
	if (listUsers->users().count() <= 0)
	{
		// default config "admin" "password"
		listUsers->addUser(
			"admin",
			QByteArray::fromHex("f5ae6aba9a6d1465b945e592340ea22358fdc24ac856d73f51a9c766cc4e69881c72a50290dfbf3d0986dd24cb4f6af6a7c0b564ad757f781339a36de93f46b4")
		);
		// set as root user
		auto adminUser = listUsers->user("admin");
		Q_CHECK_PTR(adminUser);
		Q_CHECK_PTR(server.nodeById<QUaUser>("ns=1;s=users/admin"));
		accessControl->setRootUser(adminUser);
		// permissions will be auto created later
		QObject::connect(adminUser, &QUaUser::permissionsObjectChanged,
		[adminUser, accessControl](QUaPermissions * adminOnly) {
			Q_CHECK_PTR(adminOnly);
			// set access control permissions
			accessControl->setPermissionsObject(adminOnly);		
		});
	}

	// disable anon login
	server.setAnonymousLoginAllowed(false);
	// define custom user validation 
	server.setUserValidationCallback(
		[listUsers](const QString &strUserName, const QString &strPassword) {
		auto user = listUsers->user(strUserName);
		if (!user)
		{
			return false;
		}
		return user->isPasswordValid(strPassword);
	});

	server.start();

	return a.exec(); 
}
