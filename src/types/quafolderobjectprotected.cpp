#include "quafolderobjectprotected.h"

#include <QUaServer>
#include <QUaPermissions>

QUaFolderObjectProtected::QUaFolderObjectProtected(QUaServer *server)
	: QUaFolderObject(server)
{

}

QString QUaFolderObjectProtected::setPermissions(QList<QString> strPermissionsPath)
{
	// get target node
	QUaNode * node = this->server()->browsePath(strPermissionsPath);
	QUaPermissions * permissions = dynamic_cast<QUaPermissions*>(node);
	if (!permissions)
	{
		node = this->server()->objectsFolder()->browsePath(strPermissionsPath);
		permissions = dynamic_cast<QUaPermissions*>(node);
	}
	if (!permissions)
	{
		return tr("%1 : Unexisting node in browse path %2.")
			.arg("Error")
			.arg(strPermissionsPath.join("/"));
	}
	// use c++ api
	this->setPermissionsObject(permissions);
	// return
	return "Success";
}

void QUaFolderObjectProtected::clearPermissions()
{
	// clear reference
	auto listRefs = this->findReferences(QUaPermissions::HasPermissionsRefType);
	Q_ASSERT_X(listRefs.count() <= 1, "QUaFolderObjectProtected::clearPermissions",
		"Only one permissions object per protected object is currently suppported.");
	if (listRefs.count() <= 0)
	{
		return;
	}
	this->removeReference(QUaPermissions::HasPermissionsRefType, listRefs.at(0));
	// emit
	emit this->permissionsObjectChanged(nullptr);
	// return
	return;
}

bool QUaFolderObjectProtected::hasPermissionsObject() const
{
	return this->permissionsObject();
}

QUaPermissions * QUaFolderObjectProtected::permissionsObject() const
{
	auto listRefs = this->findReferences(QUaPermissions::HasPermissionsRefType);
	Q_ASSERT_X(listRefs.count() <= 1, "QUaFolderObjectProtected::permissionsObject", 
		"Only one permissions object per protected object is currently suppported.");
	return listRefs.count() >= 1 ? dynamic_cast<QUaPermissions*>(listRefs.at(0)) : nullptr;
}

void QUaFolderObjectProtected::setPermissionsObject(QUaPermissions * permissions)
{
	// remove old reference if any
	if (this->hasPermissionsObject())
	{
		this->clearPermissions();
	}
	Q_ASSERT(!this->hasPermissionsObject());
	// add reference
	this->addReference(QUaPermissions::HasPermissionsRefType, permissions);
	// emit
	emit this->permissionsObjectChanged(permissions);
}

QUaAccessLevel QUaFolderObjectProtected::userAccessLevel(const QString & strUserName)
{
	QUaAccessLevel access;
	// If no permissions set, return default
	if (!this->hasPermissionsObject())
	{
		return QUaFolderObject::userAccessLevel(strUserName);
	}
	// Get permissions
	auto permissions = this->permissionsObject();
	Q_CHECK_PTR(permissions);
	// Set permissions according to user
	access.bits.bRead  = permissions->canUserRead (strUserName);
	access.bits.bWrite = permissions->canUserWrite(strUserName);
	return access;
}

bool QUaFolderObjectProtected::userExecutable(const QString & strUserName)
{
	// If no permissions set, return default
	if (!this->hasPermissionsObject())
	{
		return QUaFolderObject::userExecutable(strUserName);
	}
	// Get permissions
	auto permissions = this->permissionsObject();
	Q_CHECK_PTR(permissions);
	// Set permissions according to user
	return permissions->canUserWrite(strUserName);
}
