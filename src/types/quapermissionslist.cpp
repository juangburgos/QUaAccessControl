#include "quapermissionslist.h"

// NOTE : had to add this header because the actual implementation of QUaBaseObject::addChild is in here
//        and was getting "lnk2019 unresolved external symbol template function" without it
#include <QUaServer>
#include <QUaPermissions>
#include <QUaAccessControl>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

QUaPermissionsList::QUaPermissionsList(QUaServer *server)
	: QUaFolderObjectProtected(server)
{

}

QString QUaPermissionsList::addPermissions(QString strId)
{
	// validate id
	strId = strId.trimmed();
	// check empty
	if (strId.isEmpty())
	{
		return tr("%1 : Permissions Id argument cannot be empty.\n").arg("Error");
	}
	// check valid length
	if (strId.count() > 16)
	{
		return  tr("%1 : Permissions Id cannot contain more than 16 characters.\n").arg("Error");
	}
	// check valid characters
	QRegularExpression rx("^[a-zA-Z0-9_]*$");
	QRegularExpressionMatch match = rx.match(strId, 0, QRegularExpression::PartialPreferCompleteMatch);
	if (!match.hasMatch())
	{
		return  tr("%1 : Permissions Id can only contain numbers, letters and underscores /^[a-zA-Z0-9_]*$/.\n").arg("Error");
	}
	// check if id already exists
	if (this->hasChild(strId))
	{
		return  tr("%1 : Permissions Id already exists.\n").arg("Error");
	}
	// create instance
	// TODO : set custom nodeId when https://github.com/open62541/open62541/issues/2667 fixed
	//QString strNodeId = QString("ns=1;s=%1").arg(this->nodeBrowsePath().join(".") + "." + strId);
	auto permissions = this->addChild<QUaPermissions>(/*strNodeId*/);
	permissions->setDisplayName(strId);
	permissions->setBrowseName(strId);
	// return
	return "Success";
}

void QUaPermissionsList::clear()
{
	auto permissions = this->permissionsList();
	for (int i = 0; i < permissions.count(); i++)
	{
		permissions.at(i)->remove();
	}
}

QString QUaPermissionsList::xmlConfig()
{
	QDomDocument doc;
	// set xml header
	QDomProcessingInstruction header = doc.createProcessingInstruction("xml", "version='1.0' encoding='UTF-8'");
	doc.appendChild(header);
	// convert config to xml
	auto elemPerms = this->toDomElement(doc);
	doc.appendChild(elemPerms);
	// get config
	return doc.toByteArray();
}

QString QUaPermissionsList::setXmlConfig(QString strXmlConfig)
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
	QDomElement elemPerms = doc.firstChildElement(QUaPermissionsList::staticMetaObject.className());
	if (elemPerms.isNull())
	{
		strError = tr("%1 : No Permissions list found in XML config.\n").arg("Error");
		return strError;
	}
	// load config from xml
	this->fromDomElement(elemPerms, strError);
	if (strError.isEmpty())
	{
		strError = "Success.";
	}
	return strError;
}

QList<QUaPermissions*> QUaPermissionsList::permissionsList() const
{
	return this->browseChildren<QUaPermissions>();
}

QUaPermissions * QUaPermissionsList::permission(const QString & strId) const
{
	return this->browseChild<QUaPermissions>(strId);
}

QUaAccessControl * QUaPermissionsList::accessControl() const
{
	return dynamic_cast<QUaAccessControl*>(this->parent());
}

QDomElement QUaPermissionsList::toDomElement(QDomDocument & domDoc) const
{
	// add client list element
	QDomElement elemPerms = domDoc.createElement(QUaPermissionsList::staticMetaObject.className());
	// loop users and add them
	auto permissions = this->permissionsList();
	for (int i = 0; i < permissions.count(); i++)
	{
		auto perm = permissions.at(i);
		QDomElement elemPerm = perm->toDomElement(domDoc);
		elemPerms.appendChild(elemPerm);
	}
	// return list element
	return elemPerms;
}

void QUaPermissionsList::fromDomElement(QDomElement & domElem, QString & strError)
{
	// add perm elems
	QDomNodeList listPerms = domElem.elementsByTagName(QUaPermissions::staticMetaObject.className());
	for (int i = 0; i < listPerms.count(); i++)
	{
		QDomElement elem = listPerms.at(i).toElement();
		Q_ASSERT(!elem.isNull());
		if (!elem.hasAttribute("Id"))
		{
			strError += tr("%1 : Cannot add Permissions without Id attribute. Skipping.\n").arg("Error");
			continue;
		}
		// attempt to add
		QString strId = elem.attribute("Id");
		auto strErrorPerm = this->addPermissions(strId);
		if (strErrorPerm.contains("Error"))
		{
			strError += tr("%1 : Error adding Permissions %2. Skipping.\n").arg("Error").arg(strErrorPerm);
			continue;
		}
		// set client config
		auto perm = this->browseChild<QUaPermissions>(strId);
		perm->fromDomElement(elem, strError);
	}
}
