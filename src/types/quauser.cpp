#include "quauser.h"

#include <QUaServer>
#include <QUaRole>

#include <QMessageAuthenticationCode>

QUaReference QUaUser::UserHasRoleRefType = { "HasRole", "IsRoleOf" };

QUaUser::QUaUser(QUaServer *server)
	: QUaBaseObject(server)
{
	// set defaults
	hash()->setDataType(QMetaType::QByteArray);
	// set initial conditions
	hash()->setWriteAccess(false);
	// set descriptions
	hash()->setDescription(tr("A cryptographic hash of the user data. Passwords are not stored by the server."));
}

QUaProperty * QUaUser::hash() const
{
	return this->browseChild<QUaProperty>("Hash");
}

void QUaUser::remove()
{
	this->deleteLater();
}

QString QUaUser::setPassword(QString strPassword)
{
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
	QString strMessage = this->getName();
	if (this->hasRole())
	{
		strMessage += ":" + this->role()->getName();
	}
	// create hash
	QByteArray hash = QMessageAuthenticationCode::hash(
		strMessage.toUtf8(), 
		strPassword.toUtf8(), 
		QCryptographicHash::Sha512
	);
	// set hash
	this->setHash(hash);
	// return
	return "Success";
}

QString QUaUser::setRole(QString strPassword, QList<QString> strRolePath)
{
	// validate password
	strPassword = strPassword.trimmed();
	if (!this->isPasswordValid(strPassword))
	{
		return tr("%1 : Incorrect Password. Please try again.").arg("Error");
	}
	// get target node
	QUaNode * node = this->server()->browsePath(strRolePath);
	QUaRole * role = dynamic_cast<QUaRole*>(node);
	if (!role)
	{
		node = this->server()->objectsFolder()->browsePath(strRolePath);
		role = dynamic_cast<QUaRole*>(node);
	}
	if (!role)
	{
		return tr("%1 : Unexisting node in browse path %2.")
			.arg("Error")
			.arg(strRolePath.join("/"));
	}
	// remove old reference if any
	if (this->hasRole())
	{
		this->clearRole(strPassword);
	}
	Q_ASSERT(!this->hasRole());
	// add reference
	this->addReference(QUaUser::UserHasRoleRefType, role);
	// re-set password
	this->setPassword(strPassword);
	// emit
	emit this->roleChanged(role);
	// return
	return "Success";
}

QString QUaUser::clearRole(QString strPassword)
{
	// validate password
	strPassword = strPassword.trimmed();
	if (!this->isPasswordValid(strPassword))
	{
		return tr("%1 : Incorrect Password. Please try again.").arg("Error");
	}
	// clear reference
	auto listRefs = this->findReferences(QUaUser::UserHasRoleRefType);
	Q_ASSERT_X(listRefs.count() <= 1, "QUaUser::clearRole", "Only one role per user is currently supported.");
	if (listRefs.count() <= 0)
	{
		return "Success";
	}
	this->removeReference(QUaUser::UserHasRoleRefType, listRefs.at(0));
	// re-set password
	this->setPassword(strPassword);
	// emit
	emit this->roleChanged(nullptr);
	// return
	return "Success";
}

QString QUaUser::getName() const
{
	return this->browseName();
}

QByteArray QUaUser::getHash() const
{
	return this->hash()->value().toByteArray();
}

void QUaUser::setHash(const QByteArray & hash)
{
	this->hash()->setValue(hash);
	emit this->hashChanged(hash);
}

bool QUaUser::hasRole() const
{
	return this->role() ? true : false;
}

QUaRole * QUaUser::role() const
{
	auto listRefs = this->findReferences(QUaUser::UserHasRoleRefType);
	Q_ASSERT_X(listRefs.count() <= 1, "QUaUser::role", "Only one role per user is currently supported.");
	return listRefs.count() >= 1 ? dynamic_cast<QUaRole*>(listRefs.at(0)) : nullptr;
}

bool QUaUser::isPasswordValid(const QString &strPassword) const
{
	QString strMessage = this->getName();
	if (this->hasRole())
	{
		strMessage += ":" + this->role()->getName();
	}
	// create hash
	QByteArray hash = QMessageAuthenticationCode::hash(
		strMessage.toUtf8(),
		strPassword.toUtf8(),
		QCryptographicHash::Sha512
	);
	// compare
	return hash == this->getHash();
}

QDomElement QUaUser::toDomElement(QDomDocument & domDoc) const
{
	// add element
	QDomElement elem = domDoc.createElement(QUaUser::staticMetaObject.className());
	// set all attributes
	elem.setAttribute("Name", this->getName());
	elem.setAttribute("Hash", QString(this->getHash().toHex()));
	if (this->hasRole())
	{
		elem.setAttribute("Role", this->role()->nodeBrowsePath().join("/"));
	}
	// return element
	return elem;
}

void QUaUser::fromDomElement(QDomElement & domElem, QString & strError)
{
	// NOTE : at this point name must already be set
	Q_ASSERT(this->getName().compare(domElem.attribute("Name"), Qt::CaseSensitive) == 0);
	// load hash (raw)
	this->setHash(QByteArray::fromHex(domElem.attribute("Hash").toUtf8()));
	// load role (raw)
	if (domElem.attribute("Role", "").isEmpty())
	{
		return;
	}
	auto strRolePath = domElem.attribute("Role").split("/");
	QUaNode * node = this->server()->browsePath(strRolePath);
	QUaRole * role = dynamic_cast<QUaRole*>(node);
	if (!role)
	{
		node = this->server()->objectsFolder()->browsePath(strRolePath);
		role = dynamic_cast<QUaRole*>(node);
	}
	if (!role)
	{
		strError += tr("%1 : Unexisting node in browse path %2. Could not add role to user %3.\n")
			.arg("Error")
			.arg(strRolePath.join("/"))
			.arg(this->getName());
		return;
	}
	// add reference
	this->addReference(QUaUser::UserHasRoleRefType, role);
	// NOTE : undefined behaviour when role destroyed and config was loaded from XML,
	//        because password is not available to recreate the hash.
	//        therefore, we must forbid role destruction while still has users attached
	//        or delete all users attached to role automatically when destroyed.
}
