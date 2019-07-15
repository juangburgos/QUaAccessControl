#include "quaaccesscontrol.h"

#include <QUaServer>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>

QUaReference QUaAccessControl::HasRootUserRefType = { "HasRootUser"  , "IsRootUserOf" };

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
	QString strError;
	// set to dom doc
	QDomDocument doc;
	int line, col;
	doc.setContent(strXmlConfig, &strError, &line, &col);
	if (!strError.isEmpty())
	{
		strError = tr("%1 : Invalid XML in Line %2 Column %3 Error %4.\n").arg("Error").arg(line).arg(col).arg(strError);
		return strError;
	}
	// get list of params
	QDomElement elemAc = doc.firstChildElement(QUaAccessControl::staticMetaObject.className());
	if (elemAc.isNull())
	{
		strError = tr("%1 : No AccessControl element found in XML config.\n").arg("Error");
		return strError;
	}
	// load config from xml
	this->fromDomElement(elemAc, strError);
	if (strError.isEmpty())
	{
		strError = "Success.\n";
	}
	return strError;
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

bool QUaAccessControl::hasRootUser() const
{
	return this->rootUser();
}

QUaUser * QUaAccessControl::rootUser() const
{
	auto listRefs = this->findReferences(QUaAccessControl::HasRootUserRefType);
	Q_ASSERT_X(listRefs.count() <= 1, "QUaAccessControl::rootUser", "Only one root user is currently supported.");
	return listRefs.count() >= 1 ? dynamic_cast<QUaUser*>(listRefs.at(0)) : nullptr;
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

QDomElement QUaAccessControl::toDomElement(QDomDocument & domDoc) const
{
	// add ac element
	QDomElement elemAc = domDoc.createElement(QUaAccessControl::staticMetaObject.className());
	// set permissions if any
	if (this->hasPermissionsObject())
	{
		elemAc.setAttribute("Permissions", this->permissionsObject()->nodeBrowsePath().join("/"));
	}
	// set root user if any
	if (this->hasRootUser())
	{
		elemAc.setAttribute("RootUser", this->rootUser()->nodeBrowsePath().join("/"));
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

void QUaAccessControl::fromDomElement(QDomElement & domElem, QString & strError)
{
	// NOTE : first we need to instantiate all, then configure them (due to references)
	// find roles
	QDomElement elemRoles = domElem.firstChildElement(QUaRoleList::staticMetaObject.className());
	if (elemRoles.isNull())
	{
		strError = tr("%1 : No Role list found in XML config.\n").arg("Error");
		return;
	}
	// instantiate roles
	this->roles()->fromDomElementInstantiate(elemRoles, strError);
	// find roles
	QDomElement elemUsers = domElem.firstChildElement(QUaUserList::staticMetaObject.className());
	if (elemUsers.isNull())
	{
		strError = tr("%1 : No User list found in XML config.\n").arg("Error");
		return;
	}
	// instantiate roles
	this->users()->fromDomElementInstantiate(elemUsers, strError);
	// find permissions
	QDomElement elemPerms = domElem.firstChildElement(QUaPermissionsList::staticMetaObject.className());
	if (elemPerms.isNull())
	{
		strError = tr("%1 : No Permissions list found in XML config.\n").arg("Error");
		return;
	}
	// instantiate permissions list
	this->permissions()->fromDomElementInstantiate(elemPerms, strError);

	// configure all
	this->roles      ()->fromDomElementConfigure(elemRoles, strError);
	this->users      ()->fromDomElementConfigure(elemUsers, strError);
	this->permissions()->fromDomElementConfigure(elemPerms, strError);

	// load permissions if any
	if (domElem.hasAttribute("Permissions") && !domElem.attribute("Permissions").isEmpty())
	{
		auto strPermsPath = domElem.attribute("Permissions").split("/");
		strError += this->setPermissions(strPermsPath);
	}

	// load root user if any
	if (domElem.hasAttribute("RootUser") && !domElem.attribute("RootUser").isEmpty())
	{
		auto strRootUserPath = domElem.attribute("RootUser").split("/");
		// get target node
		QUaNode * node = this->server()->browsePath(strRootUserPath);
		QUaUser * user = dynamic_cast<QUaUser*>(node);
		if (!user)
		{
			node = this->browsePath(strRootUserPath);
			user = dynamic_cast<QUaUser*>(node);
		}
		if (!user)
		{
			node = this->users()->browsePath(strRootUserPath);
			user = dynamic_cast<QUaUser*>(node);
		}
		if (!user)
		{
			strError += tr("%1 : Unexisting node in browse path %2.")
				.arg("Error")
				.arg(strRootUserPath.join("/"));
		}
		else
		{
			this->setRootUser(user);
		}
	}
}
