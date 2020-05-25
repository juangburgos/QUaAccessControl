#include "quaaccesscontrol.h"

#include <QUaServer>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>

QUaReferenceType QUaAccessControl::HasRootUserRefType = { "HasRootUser"  , "IsRootUserOf" };

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

QString QUaAccessControl::xmlConfig()
{
	QDomDocument doc;
	// set xml header
	QDomProcessingInstruction header = doc.createProcessingInstruction("xml", "version='1.0' encoding='UTF-8'");
	doc.appendChild(header);
	// convert config to xml
	auto elemAc = this->toDomElement(doc);
	doc.appendChild(elemAc);
	// get config
	return doc.toByteArray();
}

QString QUaAccessControl::setXmlConfig(QString strXmlConfig)
{
	QQueue<QUaLog> errorLogs;
	// set to dom doc
	QDomDocument doc;
	int line, col;
	QString strError;
	doc.setContent(strXmlConfig, &strError, &line, &col);
	if (!strError.isEmpty())
	{
		errorLogs << QUaLog(
			tr("Invalid XML in Line %1 Column %2 Error %3.").arg(line).arg(col).arg(strError),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return QUaLog::toString(errorLogs);
	}
	// get list of params
	QDomElement elemAc = doc.firstChildElement(QUaAccessControl::staticMetaObject.className());
	if (elemAc.isNull())
	{
		errorLogs << QUaLog(
			tr("No AccessControl element found in XML config."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return QUaLog::toString(errorLogs);
	}
	// load config from xml
	this->fromDomElement(elemAc, errorLogs);
	if (!strError.isEmpty())
	{
		return QUaLog::toString(errorLogs);
	}
	return "Success.";
}

void QUaAccessControl::clear()
{
	this->permissions()->clear();
	this->roles()->clear();
	this->users()->clear();
}

QUaUserList * QUaAccessControl::users() const
{
	return const_cast<QUaAccessControl*>(this)->browseChild<QUaUserList>("Users");
}

QUaRoleList * QUaAccessControl::roles() const
{
	return const_cast<QUaAccessControl*>(this)->browseChild<QUaRoleList>("Roles");
}

QUaPermissionsList * QUaAccessControl::permissions() const
{
	return const_cast<QUaAccessControl*>(this)->browseChild<QUaPermissionsList>("Permissions");
}

bool QUaAccessControl::hasRootUser() const
{
	return this->rootUser();
}

QUaUser * QUaAccessControl::rootUser() const
{
	auto listRefs = this->findReferences(QUaAccessControl::HasRootUserRefType);
	Q_ASSERT_X(listRefs.count() <= 1, "QUaAccessControl::rootUser", "Only one root user is currently supported.");
	return listRefs.count() >= 1 ? qobject_cast<QUaUser*>(listRefs.at(0)) : nullptr;
}

void QUaAccessControl::setRootUser(QUaUser * rootUser)
{
	// remove old reference if any
	if (this->hasRootUser())
	{
		this->clearRootUser();
	}
	Q_ASSERT(!this->hasRootUser());
	// check if new role valid
	if (!rootUser)
	{
		// NOTE : clearRole already emits nullptr
		return;
	}
	// add reference
	this->addReference(QUaAccessControl::HasRootUserRefType, rootUser);
	// emit
	emit this->rootUserChanged(rootUser);
}

void QUaAccessControl::clearRootUser()
{
	// clear reference
	auto rootUser = this->rootUser();
	if (!rootUser)
	{
		return;
	}
	this->removeReference(QUaAccessControl::HasRootUserRefType, rootUser);
	// emit
	emit this->rootUserChanged(nullptr);
	// return
	return;
}

void QUaAccessControl::clearInmediatly()
{
	auto permsList = this->permissions()->permissionsList();
	for (auto perms : permsList)
	{
		delete perms;
	}
	auto roles = this->roles()->roles();
	for (auto role : roles)
	{
		delete role;
	}
	auto users = this->users()->users();
	for (auto user : users)
	{
		delete user;
	}
}

QDomElement QUaAccessControl::toDomElement(QDomDocument & domDoc) const
{
	// add ac element
	QDomElement elemAc = domDoc.createElement(QUaAccessControl::staticMetaObject.className());
	// set permissions if any
	if (this->hasPermissionsObject())
	{
		elemAc.setAttribute("Permissions", this->permissionsObject()->nodeId());
	}
	// set root user if any
	if (this->hasRootUser())
	{
		elemAc.setAttribute("RootUser", this->rootUser()->nodeId());
	}
	// attach roles, users and permissions
	QDomElement elemRoles = this->roles()->toDomElement(domDoc);
	elemAc.appendChild(elemRoles);
	QDomElement elemUsers = this->users()->toDomElement(domDoc);
	elemAc.appendChild(elemUsers);
	QDomElement elemPerms = this->permissions()->toDomElement(domDoc);
	elemAc.appendChild(elemPerms);
	// return ac element
	return elemAc;
}

void QUaAccessControl::fromDomElement(QDomElement & domElem, QQueue<QUaLog>& errorLogs)
{
	// NOTE : first we need to instantiate all, then configure them (due to references)
	// find roles
	QDomElement elemRoles = domElem.firstChildElement(QUaRoleList::staticMetaObject.className());
	if (elemRoles.isNull())
	{
		errorLogs << QUaLog(
			tr("No Role list found in XML config."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return;
	}
	// instantiate roles
	this->roles()->fromDomElementInstantiate(elemRoles, errorLogs);
	// find roles
	QDomElement elemUsers = domElem.firstChildElement(QUaUserList::staticMetaObject.className());
	if (elemUsers.isNull())
	{
		errorLogs << QUaLog(
			tr("No User list found in XML config."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return;
	}
	// instantiate roles
	this->users()->fromDomElementInstantiate(elemUsers, errorLogs);
	// find permissions
	QDomElement elemPerms = domElem.firstChildElement(QUaPermissionsList::staticMetaObject.className());
	if (elemPerms.isNull())
	{
		errorLogs << QUaLog(
			tr("No Permissions list found in XML config."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return;
	}
	// instantiate permissions list
	this->permissions()->fromDomElementInstantiate(elemPerms, errorLogs);

	// configure all
	this->roles      ()->fromDomElementConfigure(elemRoles, errorLogs);
	this->users      ()->fromDomElementConfigure(elemUsers, errorLogs);
	this->permissions()->fromDomElementConfigure(elemPerms, errorLogs);

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

	// load root user if any
	if (domElem.hasAttribute("RootUser") && !domElem.attribute("RootUser").isEmpty())
	{
		// get target node
		QUaNode * node = this->server()->nodeById(domElem.attribute("RootUser"));
		if (!node)
		{
			errorLogs << QUaLog(
				tr("Unexisting node with NodeId %1.").arg(domElem.attribute("RootUser")),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);			
			return;
		}
		QUaUser * user = qobject_cast<QUaUser*>(node);
		if (!user)
		{
			errorLogs << QUaLog(
				tr("Node with NodeId %1 is not a user.").arg(domElem.attribute("RootUser")),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
		}
		else
		{
			this->setRootUser(user);
		}
	}
}
