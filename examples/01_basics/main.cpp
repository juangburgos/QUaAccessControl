#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

#include <QUaAccessControl>
#include <QUaUser>
#include <QUaPermissions>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	QUaFolderObject * objsFolder = server.objectsFolder();

	// setup access control information model
	auto accessControl = objsFolder->addChild<QUaAccessControl>("AccessControl");

	// default admin user and hash
	// hash generated with https://www.freeformatter.com/hmac-generator.html, 
	// using sha512, string is username (e.g. "admin"), key is password (e.g. "password")
	auto listUsers = accessControl->users();
	listUsers->addUser(
		"admin", 
		QByteArray::fromHex("f5ae6aba9a6d1465b945e592340ea22358fdc24ac856d73f51a9c766cc4e69881c72a50290dfbf3d0986dd24cb4f6af6a7c0b564ad757f781339a36de93f46b4")
	);
	auto adminUser = listUsers->user("admin");
	Q_CHECK_PTR(adminUser);

	// add some other user (with no role) "juan", "whatever"
	listUsers->addUser(
		"juan",
		QByteArray::fromHex("0acca173e954e140fb621a08282b51d4df7d86ec9c916292b9fc56660451bc10bb9eaa14e02fd1a90c03722d6568c98659313e61b64bdfc6ddb29c124b8ce6e7")
	);

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

	server.start();

	return a.exec(); 
}
