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
	// check if nodeId exists
	QUaNodeId nodeId = { 0, QString("roles.%1").arg(strName) };
	if (this->server()->nodeById(nodeId))
	{
		return  tr("%1 : NodeId %2 already exists.\n").arg("Error").arg(nodeId);
	}
	// create instance
	auto role = this->addChild<QUaRole>(strName, nodeId);
	// check
	Q_ASSERT_X(role, "addRole", "Is NodeId repeated or invalid?");
	if (!role)
	{
		return tr("%1 : Failed to create role %2 with NodeId %3.\n").arg("Error").arg(strName).arg(nodeId);
	}
	// return
	return "Success\n";
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
	QDomElement elemRoles = doc.firstChildElement(QUaRoleList::staticMetaObject.className());
	if (elemRoles.isNull())
	{
		errorLogs << QUaLog(
			tr("No User list found in XML config."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return QUaLog::toString(errorLogs);
	}
	// load config from xml
	this->fromDomElementInstantiate(elemRoles, errorLogs);
	this->fromDomElementConfigure  (elemRoles, errorLogs);
	if (!strError.isEmpty())
	{
		return QUaLog::toString(errorLogs);
	}
	return "Success.";
}

QList<QUaRole*> QUaRoleList::roles() const
{
	return this->browseChildren<QUaRole>();
}

QUaRole * QUaRoleList::role(const QString & strName) const
{
	return const_cast<QUaRoleList*>(this)->browseChild<QUaRole>(strName);
}

QDomElement QUaRoleList::toDomElement(QDomDocument & domDoc) const
{
	// add client list element
	QDomElement elemRoles = domDoc.createElement(QUaRoleList::staticMetaObject.className());
	// set parmissions if any
	if (this->hasPermissionsObject())
	{
		elemRoles.setAttribute("Permissions", this->permissionsObject()->nodeId());
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

void QUaRoleList::fromDomElementInstantiate(QDomElement & domElem, QQueue<QUaLog>& errorLogs)
{
	// add role elems
	QDomNodeList listRoles = domElem.elementsByTagName(QUaRole::staticMetaObject.className());
	for (int i = 0; i < listRoles.count(); i++)
	{
		QDomElement elem = listRoles.at(i).toElement();
		Q_ASSERT(!elem.isNull());
		if (!elem.hasAttribute("Name"))
		{
			errorLogs << QUaLog(
				tr("Cannot add Role without Name attribute. Skipping."),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		// attempt to add
		QString strName = elem.attribute("Name");
		auto strErrorRole = this->addRole(strName);
		if (strErrorRole.contains("Error"))
		{
			errorLogs << QUaLog(
				tr("Error adding Role %1. Skipping.").arg(strErrorRole),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
		}
	}
}

void QUaRoleList::fromDomElementConfigure(QDomElement & domElem, QQueue<QUaLog>& errorLogs)
{
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
	// add role elems
	QDomNodeList listRoles = domElem.elementsByTagName(QUaRole::staticMetaObject.className());
	for (int i = 0; i < listRoles.count(); i++)
	{
		QDomElement elem = listRoles.at(i).toElement();
		Q_ASSERT(!elem.isNull());
		QString strName = elem.attribute("Name");
		// set role config
		auto role = this->browseChild<QUaRole>(strName);
		if (!role)
		{
			errorLogs << QUaLog(
				tr("Error finding Role %1. Skipping.").arg(strName),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		role->fromDomElement(elem, errorLogs);
	}
}

void QUaRoleList::on_childAdded(QUaNode * node)
{
	QUaRole * role = qobject_cast<QUaRole*>(node);
	if (!role)
	{
		return;
	}
	// NOTE : removed implemented in QUaRole::remove (destroyed signal is too late)
	// emit added
	emit this->roleAdded(role);
}