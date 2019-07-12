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
		strError = "Success.";
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

QDomElement QUaAccessControl::toDomElement(QDomDocument & domDoc) const
{
	// add ac element
	QDomElement elemAc = domDoc.createElement(QUaAccessControl::staticMetaObject.className());
	// set parmissions if any
	if (this->hasPermissionsObject())
	{
		elemAc.setAttribute("Permissions", this->permissionsObject()->nodeBrowsePath().join("/"));
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
	// load permissions if any
	if (domElem.hasAttribute("Permissions") && !domElem.attribute("Permissions").isEmpty())
	{
		auto strPermsPath = domElem.attribute("Permissions").split("/");
		strError += this->setPermissions(strPermsPath);
	}
	// NOTE : load order is important due to references
	// find roles
	QDomElement elemRoles = domElem.firstChildElement(QUaRoleList::staticMetaObject.className());
	if (elemRoles.isNull())
	{
		strError = tr("%1 : No Role list found in XML config.\n").arg("Error");
		return;
	}
	// load roles
	this->roles()->fromDomElement(elemRoles, strError);
	// find roles
	QDomElement elemUsers = domElem.firstChildElement(QUaUserList::staticMetaObject.className());
	if (elemUsers.isNull())
	{
		strError = tr("%1 : No User list found in XML config.\n").arg("Error");
		return;
	}
	// load roles
	this->users()->fromDomElement(elemUsers, strError);
	// find permissions
	QDomElement elemPerms = domElem.firstChildElement(QUaPermissionsList::staticMetaObject.className());
	if (elemPerms.isNull())
	{
		strError = tr("%1 : No Permissions list found in XML config.\n").arg("Error");
		return;
	}
	// load permissions
	this->permissions()->fromDomElement(elemPerms, strError);
}
