#include "quapermissions.h"

#include <QUaServer>
#include <QUaRole>
#include <QUaUser>
#include <QUaPermissions>
#include <QUaAccessControl>

QUaReferenceType QUaPermissions::IsReadableByRefType   = { "IsReadableBy"  , "CanRead"         };
QUaReferenceType QUaPermissions::IsWritableByRefType   = { "IsWritableBy"  , "CanWrite"        };
QUaReferenceType QUaPermissions::HasPermissionsRefType = { "HasPermissions", "IsPermissionsOf" };

QUaPermissions::QUaPermissions(QUaServer *server)
	: QUaBaseObjectProtected(server)
{

}

void QUaPermissions::remove()
{
	this->deleteLater();
	// NOTE : destroyed signal is too late
	emit this->list()->permissionsRemoved(this);
}

QString QUaPermissions::addRoleCanRead(QString strRoleNodeId)
{
	QString strError;
	auto role = this->findRole(strRoleNodeId, strError);
	if (!role)
	{
		return strError;
	}
	this->addRoleCanRead(role);
	return "Success";
}

QString QUaPermissions::removeRoleCanRead(QString strRoleNodeId)
{
	QString strError;
	auto role = this->findRole(strRoleNodeId, strError);
	if (!role)
	{
		return strError;
	}
	this->removeRoleCanRead(role);
	return "Success";
}

QString QUaPermissions::addRoleCanWrite(QString strRoleNodeId)
{
	QString strError;
	auto role = this->findRole(strRoleNodeId, strError);
	if (!role)
	{
		return strError;
	}
	this->addRoleCanWrite(role);
	return "Success";
}

QString QUaPermissions::removeRoleCanWrite(QString strRoleNodeId)
{
	QString strError;
	auto role = this->findRole(strRoleNodeId, strError);
	if (!role)
	{
		return strError;
	}
	this->removeRoleCanWrite(role);
	return "Success";
}

QString QUaPermissions::addUserCanRead(QString strUserNodeId)
{
	QString strError;
	auto user = this->findUser(strUserNodeId, strError);
	if (!user)
	{
		return strError;
	}
	this->addUserCanRead(user);
	return "Success";
}

QString QUaPermissions::removeUserCanRead(QString strUserNodeId)
{
	QString strError;
	auto user = this->findUser(strUserNodeId, strError);
	if (!user)
	{
		return strError;
	}
	this->removeUserCanRead(user);
	return "Success";
}

QString QUaPermissions::addUserCanWrite(QString strUserNodeId)
{
	QString strError;
	auto user = this->findUser(strUserNodeId, strError);
	if (!user)
	{
		return strError;
	}
	this->addUserCanWrite(user);
	return "Success";
}

QString QUaPermissions::removeUserCanWrite(QString strUserNodeId)
{
	QString strError;
	auto user = this->findUser(strUserNodeId, strError);
	if (!user)
	{
		return strError;
	}
	this->removeUserCanWrite(user);
	return "Success";
}

QString QUaPermissions::getId() const
{
	return this->browseName().name();
}

QUaRole * QUaPermissions::findRole(const QString &strRoleNodeId, QString &strError) const
{
	QUaNode * node = this->server()->nodeById(strRoleNodeId);
	if (!node)
	{
		strError += tr("%1 : Unexisting node with NodeId %2.")
			.arg("Error")
			.arg(strRoleNodeId);
		return nullptr;
	}
	QUaRole * role = qobject_cast<QUaRole*>(node);
	if (!role)
	{
		strError += tr("%1 : Node with NodeId %2 is not a role.")
			.arg("Error")
			.arg(strRoleNodeId);
		return nullptr;
	}
	return role;
}

QUaUser * QUaPermissions::findUser(const QString &strUserNodeId, QString & strError) const
{
	QUaNode * node = this->server()->nodeById(strUserNodeId);
	if (!node)
	{
		strError += tr("%1 : Unexisting node with NodeId %2.")
			.arg("Error")
			.arg(strUserNodeId);
		return nullptr;
	}
	QUaUser * user = qobject_cast<QUaUser*>(node);
	if (!user)
	{
		strError += tr("%1 : Node with NodeId %2 is not a user.")
			.arg("Error")
			.arg(strUserNodeId);
		return nullptr;
	}
	return user;
}

void QUaPermissions::addRoleCanRead(QUaRole * role)
{
	if (this->canRoleRead(role))
	{
		return;
	}
	// add reference
	this->addReference(QUaPermissions::IsReadableByRefType, role);
	// emit
	emit this->canReadRoleAdded(role);
	auto users = role->users();
	for (int i = 0; i < users.count(); i++)
	{
		auto user = users.at(i);
		if (this->canUserReadDirectly(user))
		{
			continue;
		}
		emit this->canReadUserAdded(user);
	}
}

void QUaPermissions::removeRoleCanRead(QUaRole * role)
{
	if (!this->canRoleRead(role))
	{
		return;
	}
	// remove reference
	this->removeReference(QUaPermissions::IsReadableByRefType, role);
	// emit
	emit this->canReadRoleRemoved(role);
	auto users = role->users();
	for (int i = 0; i < users.count(); i++)
	{
		auto user = users.at(i);
		if (this->canUserReadDirectly(user))
		{
			continue;
		}
		emit this->canReadUserRemoved(users.at(i));
	}
}

void QUaPermissions::addRoleCanWrite(QUaRole * role)
{
	if (this->canRoleWrite(role))
	{
		return;
	}
	// add reference
	this->addReference(QUaPermissions::IsWritableByRefType, role);
	// emit
	emit this->canWriteRoleAdded(role);
	auto users = role->users();
	for (int i = 0; i < users.count(); i++)
	{
		auto user = users.at(i);
		if (this->canUserWriteDirectly(user))
		{
			continue;
		}
		emit this->canWriteUserAdded(users.at(i));
	}
}

void QUaPermissions::removeRoleCanWrite(QUaRole * role)
{
	if (!this->canRoleWrite(role))
	{
		return;
	}
	// remove reference
	this->removeReference(QUaPermissions::IsWritableByRefType, role);
	// emit
	emit this->canWriteRoleRemoved(role);
	auto users = role->users();
	for (int i = 0; i < users.count(); i++)
	{
		auto user = users.at(i);
		if (this->canUserWriteDirectly(user))
		{
			continue;
		}
		emit this->canWriteUserRemoved(users.at(i));
	}
}

void QUaPermissions::addUserCanRead(QUaUser * user)
{
	if (this->canUserReadDirectly(user))
	{
		return;
	}
	// add reference
	this->addReference(QUaPermissions::IsReadableByRefType, user);
	// emit
	emit this->canReadUserAdded(user);
}

void QUaPermissions::removeUserCanRead(QUaUser * user)
{
	if (!this->canUserReadDirectly(user))
	{
		return;
	}
	// remove reference
	this->removeReference(QUaPermissions::IsReadableByRefType, user);
	// emit
	emit this->canReadUserRemoved(user);
}

void QUaPermissions::addUserCanWrite(QUaUser * user)
{
	if (this->canUserWriteDirectly(user))
	{
		return;
	}
	// add reference
	this->addReference(QUaPermissions::IsWritableByRefType, user);
	// emit
	emit this->canWriteUserAdded(user);
}

void QUaPermissions::removeUserCanWrite(QUaUser * user)
{
	if (!this->canUserWriteDirectly(user))
	{
		return;
	}
	// remove reference
	this->removeReference(QUaPermissions::IsWritableByRefType, user);
	// emit
	emit this->canWriteUserRemoved(user);
}

QList<QUaRole*> QUaPermissions::rolesCanRead() const
{
	return this->findReferences<QUaRole>(QUaPermissions::IsReadableByRefType);
}

QList<QUaRole*> QUaPermissions::rolesCanWrite() const
{
	return this->findReferences<QUaRole>(QUaPermissions::IsWritableByRefType);
}

QList<QUaUser*> QUaPermissions::usersCanRead() const
{
	QList<QUaUser*> users = this->usersCanReadDirectly();
	auto roles = this->rolesCanRead();
	for (int i = 0; i < roles.count(); i++)
	{
		users << roles.at(i)->users();
	}
	// add root user if any
	auto ac = this->list()->accessControl();
	if (ac->hasRootUser())
	{
		users << ac->rootUser();
	}
	return users.toSet().toList();
}

QList<QUaUser*> QUaPermissions::usersCanWrite() const
{
	QList<QUaUser*> users = this->usersCanWriteDirectly();
	auto roles = this->rolesCanWrite();
	for (int i = 0; i < roles.count(); i++)
	{
		users << roles.at(i)->users();
	}
	// add root user if any
	auto ac = this->list()->accessControl();
	if (ac->hasRootUser())
	{
		users << ac->rootUser();
	}
	return users.toSet().toList();
}

bool QUaPermissions::canRoleRead(QUaRole * role) const
{
	return role ? this->rolesCanRead().contains(role) : false;
}

bool QUaPermissions::canRoleWrite(QUaRole * role) const
{
	return role ? this->rolesCanWrite().contains(role) : false;
}

bool QUaPermissions::canUserRead(QUaUser * user) const
{
	// always consider root user
	return user ? this->canUserReadDirectly(user) || this->canRoleRead(user->role()) || user->isRootUser() : false;
}

bool QUaPermissions::canUserWrite(QUaUser * user) const
{
	// always consider root user
	return user ? this->canUserWriteDirectly(user) || this->canRoleWrite(user->role()) || user->isRootUser() : false;
}

bool QUaPermissions::canRoleRead(const QString strRoleName) const
{
	auto list = this->list();
	Q_CHECK_PTR(list);
	auto ac = list->accessControl();
	Q_CHECK_PTR(ac);
	auto roles = ac->roles();
	Q_CHECK_PTR(roles);
	auto role = roles->role(strRoleName);
	return this->canRoleRead(role);
}

bool QUaPermissions::canRoleWrite(const QString strRoleName) const
{
	auto list = this->list();
	Q_CHECK_PTR(list);
	auto ac = list->accessControl();
	Q_CHECK_PTR(ac);
	auto roles = ac->roles();
	Q_CHECK_PTR(roles);
	auto role = roles->role(strRoleName);
	return this->canRoleWrite(role);
}

bool QUaPermissions::canUserRead(const QString strUserName) const
{
	auto list = this->list();
	Q_CHECK_PTR(list);
	auto ac = list->accessControl();
	Q_CHECK_PTR(ac);
	auto users = ac->users();
	Q_CHECK_PTR(users);
	auto user = users->user(strUserName);
	return this->canUserRead(user);
}

bool QUaPermissions::canUserWrite(const QString strUserName) const
{
	auto list = this->list();
	Q_CHECK_PTR(list);
	auto ac = list->accessControl();
	Q_CHECK_PTR(ac);
	auto users = ac->users();
	Q_CHECK_PTR(users);
	auto user = users->user(strUserName);
	return this->canUserWrite(user);
}

QUaPermissionsList * QUaPermissions::list() const
{
	return qobject_cast<QUaPermissionsList*>(this->parent());
}

QDomElement QUaPermissions::toDomElement(QDomDocument & domDoc) const
{
	// add element
	QDomElement elem = domDoc.createElement(QUaPermissions::staticMetaObject.className());
	// set parmissions if any
	if (this->hasPermissionsObject())
	{
		elem.setAttribute("Permissions", this->permissionsObject()->nodeId());
	}
	// set all attributes
	elem.setAttribute("Id", this->getId());
	// add element for read list
	QDomElement elemReadList = domDoc.createElement("CanReadList");
	elem.appendChild(elemReadList);
	// roles can read
	auto rolesRead = this->rolesCanRead();
	if (rolesRead.count() > 0)
	{
		for (int i = 0; i < rolesRead.count(); i++)
		{
			QDomElement elemRead = domDoc.createElement("Role");
			elemRead.setAttribute("NodeId", rolesRead.at(i)->nodeId());
			elemReadList.appendChild(elemRead);
		}
	}
	// users can read directly
	auto usersRead = this->usersCanReadDirectly();
	if (usersRead.count() > 0)
	{
		for (int i = 0; i < usersRead.count(); i++)
		{
			QDomElement elemRead = domDoc.createElement("User");
			elemRead.setAttribute("NodeId", usersRead.at(i)->nodeId());
			elemReadList.appendChild(elemRead);
		}
	}
	// add element for read list
	QDomElement elemWriteList = domDoc.createElement("CanWriteList");
	elem.appendChild(elemWriteList);
	// roles can write
	auto rolesWrite = this->rolesCanWrite();
	if (rolesWrite.count() > 0)
	{
		for (int i = 0; i < rolesWrite.count(); i++)
		{
			QDomElement elemWrite = domDoc.createElement("Role");
			elemWrite.setAttribute("NodeId", rolesWrite.at(i)->nodeId());
			elemWriteList.appendChild(elemWrite);
		}
	}
	// users can write directly
	auto usersWrite = this->usersCanWriteDirectly();
	if (usersWrite.count() > 0)
	{
		for (int i = 0; i < usersWrite.count(); i++)
		{
			QDomElement elemWrite = domDoc.createElement("User");
			elemWrite.setAttribute("NodeId", usersWrite.at(i)->nodeId());
			elemWriteList.appendChild(elemWrite);
		}
	}
	// return element
	return elem;
}

void QUaPermissions::fromDomElement(QDomElement & domElem, QQueue<QUaLog>& errorLogs)
{
	// NOTE : at this point id must already be set
	Q_ASSERT(this->getId().compare(domElem.attribute("Id"), Qt::CaseSensitive) == 0);
	// load permissions if any
	if (domElem.hasAttribute("Permissions") && !domElem.attribute("Permissions").isEmpty())
	{
		QString strError = this->setPermissions(domElem.attribute("Permissions"));
		if (strError.contains("Error"))
		{
			errorLogs << QUaLog(
				strError,
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
		}
	}
	// can read
	QDomElement elemReadList = domElem.firstChildElement("CanReadList");
	if (!elemReadList.isNull())
	{
		// can read roles
		QDomNodeList listReadRoles = elemReadList.elementsByTagName("Role");
		for (int i = 0; i < listReadRoles.count(); i++)
		{
			QDomElement elem = listReadRoles.at(i).toElement();
			Q_ASSERT(!elem.isNull());
			QString strError = this->addRoleCanRead(elem.attribute("NodeId"));
			if (strError.contains("Error"))
			{
				errorLogs << QUaLog(
					strError,
					QUaLogLevel::Error,
					QUaLogCategory::Serialization
				);
			}
		}
		// can read users
		QDomNodeList listReadUsers = elemReadList.elementsByTagName("User");
		for (int i = 0; i < listReadUsers.count(); i++)
		{
			QDomElement elem = listReadUsers.at(i).toElement();
			Q_ASSERT(!elem.isNull());
			QString strError = this->addUserCanRead(elem.attribute("NodeId"));
			if (strError.contains("Error"))
			{
				errorLogs << QUaLog(
					strError,
					QUaLogLevel::Error,
					QUaLogCategory::Serialization
				);
			}
		}
	}
	// can write
	QDomElement elemWriteList = domElem.firstChildElement("CanWriteList");
	if (!elemWriteList.isNull())
	{
		// can write roles
		QDomNodeList listWriteRoles = elemWriteList.elementsByTagName("Role");
		for (int i = 0; i < listWriteRoles.count(); i++)
		{
			QDomElement elem = listWriteRoles.at(i).toElement();
			Q_ASSERT(!elem.isNull());
			QString strError = this->addRoleCanWrite(elem.attribute("NodeId"));
			if (strError.contains("Error"))
			{
				errorLogs << QUaLog(
					strError,
					QUaLogLevel::Error,
					QUaLogCategory::Serialization
				);
			}
		}
		// can write users
		QDomNodeList listWriteUsers = elemWriteList.elementsByTagName("User");
		for (int i = 0; i < listWriteUsers.count(); i++)
		{
			QDomElement elem = listWriteUsers.at(i).toElement();
			Q_ASSERT(!elem.isNull());
			QString strError = this->addUserCanWrite(elem.attribute("NodeId"));
			if (strError.contains("Error"))
			{
				errorLogs << QUaLog(
					strError,
					QUaLogLevel::Error,
					QUaLogCategory::Serialization
				);
			}
		}
	}
}

QList<QUaUser*> QUaPermissions::usersCanReadDirectly() const
{
	return this->findReferences<QUaUser>(QUaPermissions::IsReadableByRefType);
}

QList<QUaUser*> QUaPermissions::usersCanWriteDirectly() const
{
	return this->findReferences<QUaUser>(QUaPermissions::IsWritableByRefType);
}

bool QUaPermissions::canUserReadDirectly(QUaUser * user) const
{
	return user ? this->usersCanReadDirectly().contains(user) : false;
}

bool QUaPermissions::canUserWriteDirectly(QUaUser * user) const
{
	return user ? this->usersCanWriteDirectly().contains(user) : false;
}
