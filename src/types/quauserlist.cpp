#include "quauserlist.h"

// NOTE : had to add this header because the actual implementation of QUaBaseObject::addChild is in here
//        and was getting "lnk2019 unresolved external symbol template function" without it
#include <QUaServer>
#include <QUaUser>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

QUaUserList::QUaUserList(QUaServer *server)
	: QUaFolderObject(server)
{

}

QString QUaUserList::addUser(QString strName, QString strPassword)
{
	// validate name
	strName = strName.trimmed();
	// check empty
	if (strName.isEmpty())
	{
		return tr("%1 : User Name argument cannot be empty.").arg("Error");
	}
	// check valid length
	if (strName.count() > 16)
	{
		return  tr("%1 : User Name cannot contain more than 16 characters.").arg("Error");
	}
	// check valid characters
	QRegularExpression rx("^[a-zA-Z0-9_]*$");
	QRegularExpressionMatch match = rx.match(strName, 0, QRegularExpression::PartialPreferCompleteMatch);
	if (!match.hasMatch())
	{
		return  tr("%1 : User Name can only contain numbers, letters and underscores /^[a-zA-Z0-9_]*$/.").arg("Error");
	}
	// check if id already exists
	if (this->hasChild(strName))
	{
		return  tr("%1 : User Name already exists.").arg("Error");
	}
	// validate password
	strPassword = strPassword.trimmed();
	// check empty
	if (strPassword.isEmpty())
	{
		return tr("%1 : User Password argument cannot be empty.").arg("Error");
	}
	// check valid length
	if (strPassword.count() < 6)
	{
		return  tr("%1 : User Password cannot contain less than 6 characters.").arg("Error");
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
	for (int i = 0; i < this->users().count(); i++)
	{
		this->users().at(i)->remove();
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
		strError = tr("%1 : Invalid XML in Line %2 Column %3 Error %4").arg("Error").arg(line).arg(col).arg(strError);
		return strError;
	}
	// get list of params
	QDomElement elemUsers = doc.firstChildElement(QUaUserList::staticMetaObject.className());
	if (elemUsers.isNull())
	{
		strError = tr("%1 : No User list found in XML config.").arg("Error");
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

QList<QUaUser*> QUaUserList::users() const
{
	return this->browseChildren<QUaUser>();
}

QDomElement QUaUserList::toDomElement(QDomDocument & domDoc) const
{
	// add client list element
	QDomElement elemUsers = domDoc.createElement(QUaUserList::staticMetaObject.className());
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
		// NOTE : cannot use QUaUserList::addUser because server does not store passwords
		auto user = this->addChild<QUaUser>(/*strNodeId*/);
		user->setDisplayName(strName);
		user->setBrowseName(strName);
		// set client config
		user->fromDomElement(elem, strError);
	}
}
