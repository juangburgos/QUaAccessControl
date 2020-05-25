#include "quauserlist.h"

// NOTE : had to add this header because the actual implementation of QUaBaseObject::addChild is in here
//        and was getting "lnk2019 unresolved external symbol template function" without it
#include <QUaServer>
#include <QUaAccessControl>
#include <QUaUser>
#include <QUaPermissions>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

QUaUserList::QUaUserList(QUaServer *server)
	: QUaFolderObjectProtected(server)
{
	// NOTE : need to be queued, otherwise browseName will not be yet set on callback
	QObject::connect(this, &QUaNode::childAdded, this, &QUaUserList::on_childAdded, Qt::QueuedConnection);
}

QString QUaUserList::addUser(QString strName, QString strPassword)
{
	QQueue<QUaLog> errorLogs;
	if (!this->isUserNameValid(strName, errorLogs))
	{
		return QUaLog::toString(errorLogs);
	}
	// validate password
	strPassword = strPassword.trimmed();
	// check empty
	if (strPassword.isEmpty())
	{
		errorLogs << QUaLog(
			tr("User Password argument cannot be empty."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return QUaLog::toString(errorLogs);
	}
	// check valid length
	if (strPassword.count() < 6)
	{
		errorLogs << QUaLog(
			tr("User Password cannot contain less than 6 characters."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return QUaLog::toString(errorLogs);
	}
	// check if nodeId exists
	QUaNodeId nodeId = { 0, QString("users.%1").arg(strName) };
	if (this->server()->nodeById(nodeId))
	{
		errorLogs << QUaLog(
			tr("NodeId %1 already exists.").arg(nodeId),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return QUaLog::toString(errorLogs);
	}
	// create instance
	auto user = this->addChild<QUaUser>(strName, nodeId);
	// check
	Q_ASSERT_X(user, "addUser", "Is NodeId repeated or invalid?");
	if (!user)
	{
		errorLogs << QUaLog(
			tr("Failed to create user %1 with NodeId %2.").arg(strName).arg(nodeId),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return QUaLog::toString(errorLogs);
	}
	// set password
	user->setPassword(strPassword);
	// return
	return "Success\n";
}

void QUaUserList::clear()
{
	auto users = this->users();
	for (int i = 0; i < users.count(); i++)
	{
		users.at(i)->remove();
	}
}

QString QUaUserList::xmlConfig()
{
	QDomDocument doc;
	// set xml header
	QDomProcessingInstruction header = doc.createProcessingInstruction("xml", "version='1.0' encoding='UTF-8'");
	doc.appendChild(header);
	// convert config to xml
	auto elemUsers = this->toDomElement(doc);
	doc.appendChild(elemUsers);
	// get config
	return doc.toByteArray();
}

QString QUaUserList::setXmlConfig(QString strXmlConfig)
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
	QDomElement elemUsers = doc.firstChildElement(QUaUserList::staticMetaObject.className());
	if (elemUsers.isNull())
	{
		errorLogs << QUaLog(
			tr("No User list found in XML config."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return QUaLog::toString(errorLogs);
	}
	// load config from xml
	this->fromDomElementInstantiate(elemUsers, errorLogs);
	this->fromDomElementConfigure  (elemUsers, errorLogs);
	if (!errorLogs.isEmpty())
	{
		QUaLog::toString(errorLogs);
	}
	return "Success.";
}

QQueue<QUaLog> QUaUserList::addUser(const QString & strName, const QByteArray & bytaHash)
{
	QQueue<QUaLog> errorLogs;
	QString strNameCopy = strName;
	if (!this->isUserNameValid(strNameCopy, errorLogs))
	{
		return errorLogs;
	}
	QUaNodeId strNodeId = { 0, QString("users.%1").arg(strName) };
	auto user = this->addChild<QUaUser>(strName, strNodeId);
	// check
	Q_ASSERT_X(user, "addUser", "Is NodeId repeated or invalid?");
	if (!user)
	{
		errorLogs << QUaLog(
			tr("Failed to create user %1 with NodeId %2.").arg(strName).arg(strNodeId),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
	}
	// set password
	user->setHash(bytaHash);
	// return
	return errorLogs;
}

QList<QUaUser*> QUaUserList::users() const
{
	return this->browseChildren<QUaUser>();
}

QUaUser * QUaUserList::user(const QString & strName) const
{
	return const_cast<QUaUserList*>(this)->browseChild<QUaUser>(strName);
}

QUaAccessControl * QUaUserList::accessControl() const
{
	return qobject_cast<QUaAccessControl*>(this->parent());
}

QDomElement QUaUserList::toDomElement(QDomDocument & domDoc) const
{
	// add user list element
	QDomElement elemUsers = domDoc.createElement(QUaUserList::staticMetaObject.className());
	// set parmissions if any
	if (this->hasPermissionsObject())
	{
		elemUsers.setAttribute("Permissions", this->permissionsObject()->nodeId());
	}
	// loop users and add them
	auto users = this->users();
	for (int i = 0; i < users.count(); i++)
	{
		auto user = users.at(i);
		QDomElement elemUser = user->toDomElement(domDoc);
		elemUsers.appendChild(elemUser);
	}
	// return list element
	return elemUsers;
}

void QUaUserList::fromDomElementInstantiate(QDomElement & domElem, QQueue<QUaLog>& errorLogs)
{
	// add user elems
	QDomNodeList listUsers = domElem.elementsByTagName(QUaUser::staticMetaObject.className());
	for (int i = 0; i < listUsers.count(); i++)
	{
		QDomElement elem = listUsers.at(i).toElement();
		Q_ASSERT(!elem.isNull());
		if (!elem.hasAttribute("Name"))
		{
			errorLogs << QUaLog(
				tr("Cannot add User without Name attribute. Skipping."),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		if (!elem.hasAttribute("Hash"))
		{
			errorLogs << QUaLog(
				tr("Cannot add User without Hash attribute. Skipping."),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		// attempt to add
		QString strName = elem.attribute("Name");
		// validate
		if (!this->isUserNameValid(strName, errorLogs))
		{
			continue;
		}
		// NOTE : cannot use QUaUserList::addUser because server does not store passwords
		QUaNodeId nodeId = { 0, QString("users.%1").arg(strName) };
		auto user = this->addChild<QUaUser>(strName, nodeId);
		// check
		Q_ASSERT_X(user, "addUser", "Is NodeId repeated or invalid?");
		if (!user)
		{
			errorLogs << QUaLog(
				tr("Failed to create user %1 with NodeId %2.").arg(strName).arg(nodeId),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
	}
}

void QUaUserList::fromDomElementConfigure(QDomElement & domElem, QQueue<QUaLog>& errorLogs)
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
	// add user elems
	QDomNodeList listUsers = domElem.elementsByTagName(QUaUser::staticMetaObject.className());
	for (int i = 0; i < listUsers.count(); i++)
	{
		QDomElement elem = listUsers.at(i).toElement();
		Q_ASSERT(!elem.isNull());
		// attempt to add
		QString strName = elem.attribute("Name");
		// set user config
		auto user = this->browseChild<QUaUser>(strName);
		if (!user)
		{
			errorLogs << QUaLog(
				tr("Error finding User %1. Skipping.").arg(strName),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		user->fromDomElement(elem, errorLogs);
	}
}

void QUaUserList::on_childAdded(QUaNode * node)
{
	QUaUser * user = qobject_cast<QUaUser*>(node);
	if (!user)
	{
		return;
	}
	// NOTE : removed implemented in QUaUser::remove (destroyed signal is too late)
	// emit added
	emit this->userAdded(user);
}

bool QUaUserList::isUserNameValid(QString &strName, QQueue<QUaLog>& errorLogs)
{
	// validate name
	strName = strName.trimmed();
	// check empty
	if (strName.isEmpty())
	{
		errorLogs << QUaLog(
			tr("User Name argument cannot be empty."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return false;
	}
	// check valid length
	if (strName.count() > 16)
	{
		errorLogs << QUaLog(
			tr("User Name cannot contain more than 16 characters."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return false;
	}
	// check valid characters
	QRegularExpression rx("^[a-zA-Z0-9_]*$");
	QRegularExpressionMatch match = rx.match(strName, 0, QRegularExpression::PartialPreferCompleteMatch);
	if (!match.hasMatch())
	{
		errorLogs << QUaLog(
			tr("User Name can only contain numbers, letters and underscores /^[a-zA-Z0-9_]*$/."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return false;
	}
	// check if id already exists
	if (this->hasChild(strName))
	{
		errorLogs << QUaLog(
			tr("User Name already exists."),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return false;
	}
	return true;
}