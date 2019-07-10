#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

#include <QUaAccessControl>
#include <QUaUser>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	QUaFolderObject * objsFolder = server.objectsFolder();

	// setup access control information model
	auto accessControl = objsFolder->addChild<QUaAccessControl>();
	accessControl->setDisplayName("AccessControl");
	accessControl->setBrowseName ("AccessControl");

	// default role (all permissions)
	auto listRoles = accessControl->roles();
	listRoles->addRole("admin");
	auto adminRole = listRoles->role("admin");
	Q_CHECK_PTR(adminRole);

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

	// set admin role to admin user
	adminUser->setRole(adminRole);

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

	// set access control permissions

	// TODO

	server.start();

	return a.exec(); 
}
