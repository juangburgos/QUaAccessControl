#include "quabaseobjectprotected.h"

#include <QUaServer>
#include <QUaPermissions>

QUaBaseObjectProtected::QUaBaseObjectProtected(QUaServer *server)
	: QUaBaseObject(server)
{

}

QString QUaBaseObjectProtected::setPermissions(QString strPermissionsNodeId)
{
	// get target node
	QUaNode * node = this->server()->nodeById(strPermissionsNodeId);
	if (!node)
	{
		return tr("%1 : Unexisting node with NodeId %2.")
			.arg("Error")
			.arg(strPermissionsNodeId);
	}
	QUaPermissions * permissions = dynamic_cast<QUaPermissions*>(node);
	if (!permissions)
	{
		return tr("%1 : Node with NodeId %2 is not a permissions instance.")
			.arg("Error")
			.arg(strPermissionsNodeId);
	}
	// use c++ api
	this->setPermissionsObject(permissions);
	// return
	return "Success.\n";
}

void QUaBaseObjectProtected::clearPermissions()
{
	// clear reference
	auto listRefs = this->findReferences(QUaPermissions::HasPermissionsRefType);
	Q_ASSERT_X(listRefs.count() <= 1, "QUaBaseObjectProtected::clearPermissions",
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

bool QUaBaseObjectProtected::hasPermissionsObject() const
{
	return this->permissionsObject();
}

QUaPermissions * QUaBaseObjectProtected::permissionsObject() const
{
	auto listRefs = this->findReferences(QUaPermissions::HasPermissionsRefType);
	Q_ASSERT_X(listRefs.count() <= 1, "QUaBaseObjectProtected::permissionsObject",
		"Only one permissions object per protected object is currently suppported.");
	return listRefs.count() >= 1 ? dynamic_cast<QUaPermissions*>(listRefs.at(0)) : nullptr;
}

void QUaBaseObjectProtected::setPermissionsObject(QUaPermissions * permissions)
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

QUaAccessLevel QUaBaseObjectProtected::userAccessLevel(const QString & strUserName)
{
	QUaAccessLevel access;
	// If no permissions set, return default
	if (!this->hasPermissionsObject())
	{
		return QUaBaseObject::userAccessLevel(strUserName);
	}
	// Get permissions
	auto permissions = this->permissionsObject();
	Q_CHECK_PTR(permissions);
	// Set permissions according to user
	access.bits.bRead  = permissions->canUserRead(strUserName);
	access.bits.bWrite = permissions->canUserWrite(strUserName);
	return access;
}

bool QUaBaseObjectProtected::userExecutable(const QString & strUserName)
{
	// If no permissions set, return default
	if (!this->hasPermissionsObject())
	{
		return QUaBaseObject::userExecutable(strUserName);
	}
	// Get permissions
	auto permissions = this->permissionsObject();
	Q_CHECK_PTR(permissions);
	// Set permissions according to user
	return permissions->canUserWrite(strUserName);
}
