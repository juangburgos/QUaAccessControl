#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

#include <QUaUserList>
#include <QUaRoleList>
#include <QUaPermissionsList>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	QUaFolderObject * objsFolder = server.objectsFolder();

	// add list entry point to object's folder
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
	
	server.start();

	return a.exec(); 
}
