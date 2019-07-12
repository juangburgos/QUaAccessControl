#include <QCoreApplication>
#include <QFile>
#include <QDebug>

#include <QUaServer>
#include <QUaAccessControl>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>

#include <FunctionUtils>

QString strFileName = "AcConfig.xml";

void configLoad(QUaAccessControl * ac)
{
	QFile fileConfig(strFileName);
	// check exists and can be opened
	if (!fileConfig.exists())
	{
		qWarning() << QObject::tr("File %1 does not exist.").arg(strFileName);
	}
	else if (fileConfig.open(QIODevice::ReadOnly))
	{
		// set config
		auto strError = ac->setXmlConfig(fileConfig.readAll());
		if (strError.contains("Error"))
		{
			qWarning() << strError;
		}
	}
	else
	{
		qWarning() << QObject::tr("File %1 could not be opened.").arg(strFileName);
	}
}

void configSave(QUaAccessControl * ac)
{
	// save to file
	QString strSaveError;
	QFile file(strFileName);
	if (file.open(QIODevice::ReadWrite | QFile::Truncate))
	{
		// get config
		QTextStream stream(&file);
		stream << ac->xmlConfig();
	}
	else
	{
		qCritical() << QObject::tr("Error opening file %1 for write operations.").arg(strFileName);
	}
	// close file
	file.close();
}

void autoConfigSave(QUaAccessControl * ac)
{
	// save inmediatly on any change, then wait 5 secs for any further changes and if any, save again after wait
	auto throttledConfigSave = std::bind(FunctionUtils::Throttle(&configSave, 5000), ac);
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
	QObject::connect(ac->users(), &QUaUserList::userRemoved, throttledConfigSave);
	// changes in roles list
	QObject::connect(ac->roles(), &QUaRoleList::permissionsObjectChanged, throttledConfigSave);
	QObject::connect(ac->roles(), &QUaRoleList::roleAdded  , 
	[throttledConfigSave](QUaRole * role) {
		// changes in role
		QObject::connect(role, &QUaRole::permissionsObjectChanged, throttledConfigSave);
	});
	QObject::connect(ac->roles(), &QUaRoleList::roleRemoved, throttledConfigSave);
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
	QObject::connect(ac->permissions(), &QUaPermissionsList::permissionsRemoved, throttledConfigSave);
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
	configLoad(accessControl);

	// setup auto config save
	autoConfigSave(accessControl);

	auto listUsers = accessControl->users();

	// if no users, then create default
	if (listUsers->users().count() <= 0)
	{
		// default config "admin" "password"
		listUsers->addUser(
			"admin",
			QByteArray::fromHex("f5ae6aba9a6d1465b945e592340ea22358fdc24ac856d73f51a9c766cc4e69881c72a50290dfbf3d0986dd24cb4f6af6a7c0b564ad757f781339a36de93f46b4")
		);
		auto adminUser = listUsers->user("admin");
		Q_CHECK_PTR(adminUser);

		// create "admin only" permissions object
		QString strAdminOnly = QString("only_%1").arg(adminUser->getName());
		auto listPermissions = accessControl->permissions();
		listPermissions->addPermissions(strAdminOnly);
		auto adminOnly = listPermissions->permission(strAdminOnly);
		Q_CHECK_PTR(adminOnly);
		// give admin user full permissions
		adminOnly->addUserCanWrite(adminUser);
		adminOnly->addUserCanRead (adminUser);

		// set access control permissions
		accessControl->setPermissionsObject(adminOnly);
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
