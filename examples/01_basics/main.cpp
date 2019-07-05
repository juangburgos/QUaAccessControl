#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

#include <QUaUserList>
#include <QUaRoleList>
#include <QUaPermissionsList>
#include <QUaUser>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	QUaFolderObject * objsFolder = server.objectsFolder();

	// setup access control information model
	auto accessControl = objsFolder->addFolderObject();
	accessControl->setDisplayName("AccessControl");
	accessControl->setBrowseName ("AccessControl");
	// users
	auto listUsers = accessControl->addChild<QUaUserList>();
	listUsers->setDisplayName("Users");
	listUsers->setBrowseName ("Users");
	// roles
	auto listRoles = accessControl->addChild<QUaRoleList>();
	listRoles->setDisplayName("Roles");
	listRoles->setBrowseName ("Roles");
	// permissions
	auto listPermissions = accessControl->addChild<QUaPermissionsList>();
	listPermissions->setDisplayName("Permissions");
	listPermissions->setBrowseName ("Permissions");

	// default user and hash
	// hash generated with https://www.freeformatter.com/hmac-generator.html, 
	// using sha512, string is username (e.g. "admin"), key is password (e.g. "password")
	listUsers->addUser(
		"admin", 
		QByteArray::fromHex("f5ae6aba9a6d1465b945e592340ea22358fdc24ac856d73f51a9c766cc4e69881c72a50290dfbf3d0986dd24cb4f6af6a7c0b564ad757f781339a36de93f46b4")
	);
	// disable anon login and define custom user validation
	server.setAnonymousLoginAllowed(false);
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
