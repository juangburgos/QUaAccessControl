#include "quapermissions.h"

#include <QUaServer>
#include <QUaRole>
#include <QUaUser>
#include <QUaAccessControl>

QUaReference QUaPermissions::IsReadableByRefType   = { "IsReadableBy"  , "CanRead"         };
QUaReference QUaPermissions::IsWritableByRefType   = { "IsWritableBy"  , "CanWrite"        };
QUaReference QUaPermissions::HasPermissionsRefType = { "HasPermissions", "IsPermissionsOf" };

QUaPermissions::QUaPermissions(QUaServer *server)
	: QUaBaseObjectProtected(server)
{

}

void QUaPermissions::remove()
{
	this->deleteLater();
}

QString QUaPermissions::addRoleCanRead(QList<QString> strRolePath)
{
	QString strError;
	auto role = this->findRole(strRolePath, strError);
	if (!role)
	{
		return strError;
	}
	this->addRoleCanRead(role);
	return "Success";
}

QString QUaPermissions::removeRoleCanRead(QList<QString> strRolePath)
{
	QString strError;
	auto role = this->findRole(strRolePath, strError);
	if (!role)
	{
		return strError;
	}
	this->removeRoleCanRead(role);
	return "Success";
}

QString QUaPermissions::addRoleCanWrite(QList<QString> strRolePath)
{
	QString strError;
	auto role = this->findRole(strRolePath, strError);
	if (!role)
	{
		return strError;
	}
	this->addRoleCanWrite(role);
	return "Success";
}

QString QUaPermissions::removeRoleCanWrite(QList<QString> strRolePath)
{
	QString strError;
	auto role = this->findRole(strRolePath, strError);
	if (!role)
	{
		return strError;
	}
	this->removeRoleCanWrite(role);
	return "Success";
}

QString QUaPermissions::getId() const
{
	return this->browseName();
}

QUaRole * QUaPermissions::findRole(const QList<QString> &strRolePath, QString &strError) const
{
	QUaNode * node = this->server()->browsePath(strRolePath);
	QUaRole * role = dynamic_cast<QUaRole*>(node);
	if (!role)
	{
		node = this->server()->objectsFolder()->browsePath(strRolePath);
		role = dynamic_cast<QUaRole*>(node);
	}
	if (!role)
	{
		strError += tr("%1 : Unexisting node in browse path %2.")
			.arg("Error")
			.arg(strRolePath.join("/"));
	}
	return role;
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
	QList<QUaUser*> users;
	auto roles = this->rolesCanRead();
	for (int i = 0; i < roles.count(); i++)
	{
		users << roles.at(i)->users();
	}
	return users;
}

QList<QUaUser*> QUaPermissions::usersCanWrite() const
{
	QList<QUaUser*> users;
	auto roles = this->rolesCanWrite();
	for (int i = 0; i < roles.count(); i++)
	{
		users << roles.at(i)->users();
	}
	return users;
}

bool QUaPermissions::canRoleRead(QUaRole * role) const
{
	return this->rolesCanRead().contains(role);
}

bool QUaPermissions::canRoleWrite(QUaRole * role) const
{
	return this->rolesCanWrite().contains(role);
}

bool QUaPermissions::canUserRead(QUaUser * user) const
{
	return this->canRoleRead(user->role());
}

bool QUaPermissions::canUserWrite(QUaUser * user) const
{
	return this->canRoleWrite(user->role());
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
	return dynamic_cast<QUaPermissionsList*>(this->parent());
}

QDomElement QUaPermissions::toDomElement(QDomDocument & domDoc) const
{
	// add element
	QDomElement elem = domDoc.createElement(QUaPermissions::staticMetaObject.className());
	// set all attributes
	elem.setAttribute("Id", this->getId());
	// can read
	auto rolesRead = this->rolesCanRead();
	if (rolesRead.count() > 0)
	{
		// add element for read list
		QDomElement elemReadList = domDoc.createElement("CanReadList");
		for (int i = 0; i < rolesRead.count(); i++)
		{
			QDomElement elemRead = domDoc.createElement("CanRead");
			elemRead.setAttribute("Role", rolesRead.at(i)->nodeBrowsePath().join("/"));
			elemReadList.appendChild(elemRead);
		}
		elem.appendChild(elemReadList);
	}
	// can write
	auto rolesWrite = this->rolesCanWrite();
	if (rolesWrite.count() > 0)
	{
		// add element for read list
		QDomElement elemWriteList = domDoc.createElement("CanWriteList");
		for (int i = 0; i < rolesWrite.count(); i++)
		{
			QDomElement elemWrite = domDoc.createElement("CanWrite");
			elemWrite.setAttribute("Role", rolesWrite.at(i)->nodeBrowsePath().join("/"));
			elemWriteList.appendChild(elemWrite);
		}
		elem.appendChild(elemWriteList);
	}
	// return element
	return elem;
}

void QUaPermissions::fromDomElement(QDomElement & domElem, QString & strError)
{
	// NOTE : at this point id must already be set
	Q_ASSERT(this->getId().compare(domElem.attribute("Id"), Qt::CaseSensitive) == 0);
	// can read
	QDomElement elemReadList = domElem.firstChildElement("CanReadList");
	if (!elemReadList.isNull())
	{
		QDomNodeList listReadRoles = elemReadList.elementsByTagName("CanRead");
		for (int i = 0; i < listReadRoles.count(); i++)
		{
			QDomElement elem = listReadRoles.at(i).toElement();
			Q_ASSERT(!elem.isNull());
			auto strRolePath  = elem.attribute("Role").split("/");
			strError += this->addRoleCanRead(strRolePath);
		}
	}
	// can write
	QDomElement elemWriteList = domElem.firstChildElement("CanWriteList");
	if (!elemWriteList.isNull())
	{
		QDomNodeList listWriteRoles = elemWriteList.elementsByTagName("CanWrite");
		for (int i = 0; i < listWriteRoles.count(); i++)
		{
			QDomElement elem = listWriteRoles.at(i).toElement();
			Q_ASSERT(!elem.isNull());
			auto strRolePath = elem.attribute("Role").split("/");
			strError += this->addRoleCanWrite(strRolePath);
		}
	}
}
