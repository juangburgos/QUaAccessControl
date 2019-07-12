#include "quarolelist.h"

// NOTE : had to add this header because the actual implementation of QUaBaseObject::addChild is in here
//        and was getting "lnk2019 unresolved external symbol template function" without it
#include <QUaServer>
#include <QUaRole>
#include <QUaPermissions>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

QUaRoleList::QUaRoleList(QUaServer *server)
	: QUaFolderObjectProtected(server)
{
	// NOTE : need to be queued, otherwise browseName will not be yet set on callback
	QObject::connect(this, &QUaNode::childAdded, this, &QUaRoleList::on_childAdded, Qt::QueuedConnection);
}

QString QUaRoleList::addRole(QString strName)
{
	// validate name
	strName = strName.trimmed();
	// check empty
	if (strName.isEmpty())
	{
		return tr("%1 : Role Name argument cannot be empty.\n").arg("Error");
	}
	// check valid length
	if (strName.count() > 16)
	{
		return  tr("%1 : Role Name cannot contain more than 16 characters.\n").arg("Error");
	}
	// check valid characters
	QRegularExpression rx("^[a-zA-Z0-9_]*$");
	QRegularExpressionMatch match = rx.match(strName, 0, QRegularExpression::PartialPreferCompleteMatch);
	if (!match.hasMatch())
	{
		return  tr("%1 : Role Name can only contain numbers, letters and underscores /^[a-zA-Z0-9_]*$/.\n").arg("Error");
	}
	// check if id already exists
	if (this->hasChild(strName))
	{
		return  tr("%1 : Role Name already exists.\n").arg("Error");
	}
	// create instance
	// TODO : set custom nodeId when https://github.com/open62541/open62541/issues/2667 fixed
	//QString strNodeId = QString("ns=1;s=%1").arg(this->nodeBrowsePath().join(".") + "." + strName);
	auto role = this->addChild<QUaRole>(/*strNodeId*/);
	role->setDisplayName(strName);
	role->setBrowseName(strName);
	// return
	return "Success";
}

void QUaRoleList::clear()
{
	for (int i = 0; i < this->roles().count(); i++)
	{
		this->roles().at(i)->remove();
	}
}

QString QUaRoleList::xmlConfig()
{
	QDomDocument doc;
	// set xml header
	QDomProcessingInstruction header = doc.createProcessingInstruction("xml", "version='1.0' encoding='UTF-8'");
	doc.appendChild(header);
	// convert config to xml
	auto elemRoles = this->toDomElement(doc);
	doc.appendChild(elemRoles);
	// get config
	return doc.toByteArray();
}

QString QUaRoleList::setXmlConfig(QString strXmlConfig)
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
	QDomElement elemRoles = doc.firstChildElement(QUaRoleList::staticMetaObject.className());
	if (elemRoles.isNull())
	{
		strError = tr("%1 : No User list found in XML config.\n").arg("Error");
		return strError;
	}
	// load config from xml
	this->fromDomElement(elemRoles, strError);
	if (strError.isEmpty())
	{
		strError = "Success.";
	}
	return strError;
}

QList<QUaRole*> QUaRoleList::roles() const
{
	return this->browseChildren<QUaRole>();
}

QUaRole * QUaRoleList::role(const QString & strName) const
{
	return this->browseChild<QUaRole>(strName);
}

QDomElement QUaRoleList::toDomElement(QDomDocument & domDoc) const
{
	// add client list element
	QDomElement elemRoles = domDoc.createElement(QUaRoleList::staticMetaObject.className());
	// set parmissions if any
	if (this->hasPermissionsObject())
	{
		elemRoles.setAttribute("Permissions", this->permissionsObject()->nodeBrowsePath().join("/"));
	}
	// loop users and add them
	auto roles = this->roles();
	for (int i = 0; i < roles.count(); i++)
	{
		auto role = roles.at(i);
		QDomElement elemRole = role->toDomElement(domDoc);
		elemRoles.appendChild(elemRole);
	}
	// return list element
	return elemRoles;
}

void QUaRoleList::fromDomElement(QDomElement & domElem, QString & strError)
{
	// load permissions if any
	if (domElem.hasAttribute("Permissions") && !domElem.attribute("Permissions").isEmpty())
	{
		auto strPermsPath = domElem.attribute("Permissions").split("/");
		strError += this->setPermissions(strPermsPath);
	}
	// add user elems
	QDomNodeList listRoles = domElem.elementsByTagName(QUaRole::staticMetaObject.className());
	for (int i = 0; i < listRoles.count(); i++)
	{
		QDomElement elem = listRoles.at(i).toElement();
		Q_ASSERT(!elem.isNull());
		if (!elem.hasAttribute("Name"))
		{
			strError += tr("%1 : Cannot add Role without Name attribute. Skipping.\n").arg("Error");
			continue;
		}
		// attempt to add
		QString strName = elem.attribute("Name");
		auto strErrorRole = this->addRole(strName);
		if (strErrorRole.contains("Error"))
		{
			strError += tr("%1 : Error adding Role %2. Skipping.\n").arg("Error").arg(strErrorRole);
			continue;
		}
		// set client config
		auto role = this->browseChild<QUaRole>(strName);
		role->fromDomElement(elem, strError);
	}
}

void QUaRoleList::on_childAdded(QUaNode * node)
{
	QUaRole * role = dynamic_cast<QUaRole*>(node);
	if (!role)
	{
		return;
	}
	// subscribe to remove
	QObject::connect(role, &QObject::destroyed, this,
		[this, role]() {
		// emit removed
		emit this->roleRemoved(role);
	});
	// emit added
	emit this->roleAdded(role);
}