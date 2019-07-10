#include "quaaccesscontrol.h"

#include <QUaServer>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>

QUaAccessControl::QUaAccessControl(QUaServer *server)
	: QUaFolderObjectProtected(server)
{
	server->registerType<QUaUserList>();
	server->registerType<QUaUser>();
	server->registerType<QUaRoleList>();
	server->registerType<QUaRole>();
	server->registerType<QUaPermissionsList>();
	server->registerType<QUaPermissions>();
}

QUaUserList * QUaAccessControl::users() const
{
	return this->browseChild<QUaUserList>("Users");
}

QUaRoleList * QUaAccessControl::roles() const
{
	return this->browseChild<QUaRoleList>("Roles");
}

QUaPermissionsList * QUaAccessControl::permissions() const
{
	return this->browseChild<QUaPermissionsList>("Permissions");
}
