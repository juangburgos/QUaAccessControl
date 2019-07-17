#include "quapermissionslist.h"

// NOTE : had to add this header because the actual implementation of QUaBaseObject::addChild is in here
//        and was getting "lnk2019 unresolved external symbol template function" without it
#include <QUaServer>
#include <QUaAccessControl>
#include <QUaPermissions>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

QUaPermissionsList::QUaPermissionsList(QUaServer *server)
	: QUaFolderObjectProtected(server)
{
	// NOTE : need to be queued, otherwise browseName will not be yet set on callback
	QObject::connect(this, &QUaNode::childAdded, this, &QUaPermissionsList::on_childAdded, Qt::QueuedConnection);
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
	// check valid length (userLength16 + onlyuser_ | only_role)
	if (strId.count() > 25)
	{
		return  tr("%1 : Permissions Id cannot contain more than 25 characters.\n").arg("Error");
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
	QString strNodeId = QString("ns=1;s=permissions/%1").arg(strId);
	auto permissions = this->addChild<QUaPermissions>(strNodeId);
	// check
	Q_ASSERT_X(permissions, "addPermissions", "Is NodeId repeated or invalid?");
	if (!permissions)
	{
		return tr("%1 : Failed to create permissions %2 with NodeId %3.\n").arg("Error").arg(strId).arg(strNodeId);
	}
	permissions->setDisplayName(strId);
	permissions->setBrowseName(strId);
	// return
	return "Success\n";
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
	this->fromDomElementInstantiate(elemPerms, strError);
	this->fromDomElementConfigure  (elemPerms, strError);
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
	// set parmissions if any
	if (this->hasPermissionsObject())
	{
		elemPerms.setAttribute("Permissions", this->permissionsObject()->nodeBrowsePath().join("/"));
	}
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

void QUaPermissionsList::fromDomElementInstantiate(QDomElement & domElem, QString & strError)
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
		}
	}
}

void QUaPermissionsList::fromDomElementConfigure(QDomElement & domElem, QString & strError)
{
	// load permissions if any
	if (domElem.hasAttribute("Permissions") && !domElem.attribute("Permissions").isEmpty())
	{
		auto strPermsPath = domElem.attribute("Permissions").split("/");
		strError += this->setPermissions(strPermsPath);
	}
	// add perm elems
	QDomNodeList listPerms = domElem.elementsByTagName(QUaPermissions::staticMetaObject.className());
	for (int i = 0; i < listPerms.count(); i++)
	{
		QDomElement elem = listPerms.at(i).toElement();
		Q_ASSERT(!elem.isNull());
		// set permissions config		
		QString strId = elem.attribute("Id");
		auto perm = this->browseChild<QUaPermissions>(strId);
		if (!perm)
		{
			strError += tr("%1 : Error finding Permissions %2. Skipping.\n").arg("Error").arg(strId);
			continue;
		}
		perm->fromDomElement(elem, strError);
	}
}

void QUaPermissionsList::on_childAdded(QUaNode * node)
{
	QUaPermissions * permissions = dynamic_cast<QUaPermissions*>(node);
	if (!permissions)
	{
		return;
	}
	// NOTE : removed implemented in QUaPermissions::remove (destroyed signal is too late)
	// emit added
	emit this->permissionsAdded(permissions);
}