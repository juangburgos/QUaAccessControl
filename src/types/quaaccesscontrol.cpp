#include "quaaccesscontrol.h"

QUaAccessControl::QUaAccessControl(QUaServer *server)
	: QUaFolderObject(server)
{

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
