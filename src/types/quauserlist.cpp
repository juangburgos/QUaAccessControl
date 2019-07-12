#include "quauserlist.h"

// NOTE : had to add this header because the actual implementation of QUaBaseObject::addChild is in here
//        and was getting "lnk2019 unresolved external symbol template function" without it
#include <QUaServer>
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
	QString strError;
	if (!this->isUserNameValid(strName, strError))
	{
		return strError;
	}
	// validate password
	strPassword = strPassword.trimmed();
	// check empty
	if (strPassword.isEmpty())
	{
		return tr("%1 : User Password argument cannot be empty.\n").arg("Error");
	}
	// check valid length
	if (strPassword.count() < 6)
	{
		return  tr("%1 : User Password cannot contain less than 6 characters.\n").arg("Error");
	}
	// create instance
	// TODO : set custom nodeId when https://github.com/open62541/open62541/issues/2667 fixed
	//QString strNodeId = QString("ns=1;s=%1").arg(this->nodeBrowsePath().join(".") + "." + strName);
	auto user = this->addChild<QUaUser>(/*strNodeId*/);
	user->setDisplayName(strName);
	user->setBrowseName(strName);
	// set password
	user->setPassword(strPassword);
	// return
	return "Success";
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
	QDomElement elemUsers = doc.firstChildElement(QUaUserList::staticMetaObject.className());
	if (elemUsers.isNull())
	{
		strError = tr("%1 : No User list found in XML config.\n").arg("Error");
		return strError;
	}
	// load config from xml
	this->fromDomElement(elemUsers, strError);
	if (strError.isEmpty())
	{
		strError = "Success.";
	}
	return strError;
}

QString QUaUserList::addUser(const QString & strName, const QByteArray & bytaHash)
{
	QString strError;
	QString strNameCopy = strName;
	if (!this->isUserNameValid(strNameCopy, strError))
	{
		return strError;
	}
	auto user = this->addChild<QUaUser>(/*strNodeId*/);
	user->setDisplayName(strName);
	user->setBrowseName(strName);
	// set password
	user->setHash(bytaHash);
	// return
	return "Success";
}

QList<QUaUser*> QUaUserList::users() const
{
	return this->browseChildren<QUaUser>();
}

QUaUser * QUaUserList::user(const QString & strName) const
{
	return this->browseChild<QUaUser>(strName);
}

QDomElement QUaUserList::toDomElement(QDomDocument & domDoc) const
{
	// add user list element
	QDomElement elemUsers = domDoc.createElement(QUaUserList::staticMetaObject.className());
	// set parmissions if any
	if (this->hasPermissionsObject())
	{
		elemUsers.setAttribute("Permissions", this->permissionsObject()->nodeBrowsePath().join("/"));
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

void QUaUserList::fromDomElement(QDomElement & domElem, QString & strError)
{
	// load permissions if any
	if (domElem.hasAttribute("Permissions") && !domElem.attribute("Permissions").isEmpty())
	{
		auto strPermsPath = domElem.attribute("Permissions").split("/");
		strError += this->setPermissions(strPermsPath);
	}
	// add user elems
	QDomNodeList listUsers = domElem.elementsByTagName(QUaUser::staticMetaObject.className());
	for (int i = 0; i < listUsers.count(); i++)
	{
		QDomElement elem = listUsers.at(i).toElement();
		Q_ASSERT(!elem.isNull());
		if (!elem.hasAttribute("Name"))
		{
			strError += tr("%1 : Cannot add User without Name attribute. Skipping.\n").arg("Error");
			continue;
		}
		if (!elem.hasAttribute("Hash"))
		{
			strError += tr("%1 : Cannot add User without Hash attribute. Skipping.\n").arg("Error");
			continue;
		}
		// attempt to add
		QString strName = elem.attribute("Name");
		// validate
		if (!this->isUserNameValid(strName, strError))
		{
			continue;
		}
		// NOTE : cannot use QUaUserList::addUser because server does not store passwords
		auto user = this->addChild<QUaUser>(/*strNodeId*/);
		user->setDisplayName(strName);
		user->setBrowseName(strName);
		// set client config
		user->fromDomElement(elem, strError);
	}
}

void QUaUserList::on_childAdded(QUaNode * node)
{
	QUaUser * user = dynamic_cast<QUaUser*>(node);
	if (!user)
	{
		return;
	}
	// subscribe to remove
	QObject::connect(user, &QObject::destroyed, this, 
	[this, user]() {
		// emit removed
		emit this->userRemoved(user);
	});
	// emit added
	emit this->userAdded(user);
}

bool QUaUserList::isUserNameValid(QString &strName, QString &strError)
{
	// validate name
	strName = strName.trimmed();
	// check empty
	if (strName.isEmpty())
	{
		strError += tr("%1 : User Name argument cannot be empty.\n").arg("Error");
		return false;
	}
	// check valid length
	if (strName.count() > 16)
	{
		strError += tr("%1 : User Name cannot contain more than 16 characters.\n").arg("Error");
		return false;
	}
	// check valid characters
	QRegularExpression rx("^[a-zA-Z0-9_]*$");
	QRegularExpressionMatch match = rx.match(strName, 0, QRegularExpression::PartialPreferCompleteMatch);
	if (!match.hasMatch())
	{
		strError += tr("%1 : User Name can only contain numbers, letters and underscores /^[a-zA-Z0-9_]*$/.\n").arg("Error");
		return false;
	}
	// check if id already exists
	if (this->hasChild(strName))
	{
		strError += tr("%1 : User Name already exists.\n").arg("Error");
		return false;
	}
	return true;
}