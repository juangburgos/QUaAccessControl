#include "quauser.h"

#include <QUaServer>
#include <QUaAccessControl>
#include <QUaRole>
#include <QUaPermissions>

#include <QMessageAuthenticationCode>

QUaReference QUaUser::UserHasRoleRefType = { "HasRole", "IsRoleOf" };

QUaUser::QUaUser(QUaServer *server)
	: QUaBaseObjectProtected(server)
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
	// NOTE : destroyed signal is too late
	emit this->list()->userRemoved(this);
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

QString QUaUser::setRole(QString strRoleNodeId)
{
	// check if exists
	QUaNode * node = this->server()->nodeById(strRoleNodeId);
	if (!node)
	{
		return tr("%1 : Unexisting node with NodeId %2.")
			.arg("Error")
			.arg(strRoleNodeId);
	}
	// try to cast
	QUaRole * role = dynamic_cast<QUaRole*>(node);
	if (!role)
	{
		return tr("%1 : Node with NodeId %2 is not a role.")
			.arg("Error")
			.arg(strRoleNodeId);
	}
	// use c++ api
	this->setRole(role);
	// return
	return "Success";
}

void QUaUser::clearRole()
{
	// clear reference
	auto role = this->role();
	if (!role)
	{
		return;
	}
	this->removeReference(QUaUser::UserHasRoleRefType, role);
	// emit
	emit this->roleChanged(nullptr);
	// return
	return;
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
	return this->role();
}

QUaRole * QUaUser::role() const
{
	auto listRefs = this->findReferences(QUaUser::UserHasRoleRefType);
	Q_ASSERT_X(listRefs.count() <= 1, "QUaUser::role", "Only one role per user is currently supported.");
	return listRefs.count() >= 1 ? dynamic_cast<QUaRole*>(listRefs.at(0)) : nullptr;
}

void QUaUser::setRole(QUaRole * role)
{
	// remove old reference if any
	if (this->hasRole())
	{
		this->clearRole();
	}
	Q_ASSERT(!this->hasRole());
	// check if new role valid
	if (!role)
	{
		// NOTE : clearRole already emits nullptr
		return;
	}
	// add reference
	this->addReference(QUaUser::UserHasRoleRefType, role);
	// emit
	emit this->roleChanged(role);
}

bool QUaUser::isPasswordValid(const QString &strPassword) const
{
	QString strMessage = this->getName();
	// create hash
	QByteArray hash = QMessageAuthenticationCode::hash(
		strMessage.toUtf8(),
		strPassword.toUtf8(),
		QCryptographicHash::Sha512
	);
	// compare
	return hash == this->getHash();
}

QUaUserList * QUaUser::list() const
{
	return dynamic_cast<QUaUserList*>(this->parent());
}

bool QUaUser::isRootUser() const
{
	auto listRefs = this->findReferences(QUaAccessControl::HasRootUserRefType, false);
	Q_ASSERT(listRefs.count() <= 1);
	return listRefs.count() >= 1 ? true : false;
}

QDomElement QUaUser::toDomElement(QDomDocument & domDoc) const
{
	// add element
	QDomElement elem = domDoc.createElement(QUaUser::staticMetaObject.className());
	// set parmissions if any
	if (this->hasPermissionsObject())
	{
		elem.setAttribute("Permissions", this->permissionsObject()->nodeId());
	}
	// set all attributes
	elem.setAttribute("Name", this->getName());
	elem.setAttribute("Hash", QString(this->getHash().toHex()));
	if (this->hasRole())
	{
		elem.setAttribute("Role", this->role()->nodeId());
	}
	// return element
	return elem;
}

void QUaUser::fromDomElement(QDomElement & domElem, QString & strError)
{
	// NOTE : at this point name must already be set
	Q_ASSERT(this->getName().compare(domElem.attribute("Name"), Qt::CaseSensitive) == 0);
	// load permissions if any
	if (domElem.hasAttribute("Permissions") && !domElem.attribute("Permissions").isEmpty())
	{
		strError += this->setPermissions(domElem.attribute("Permissions"));
	}
	// load hash (raw)
	this->setHash(QByteArray::fromHex(domElem.attribute("Hash").toUtf8()));
	// load role (raw)
	if (domElem.attribute("Role", "").isEmpty())
	{
		// having no role is acceptable, so no error or warning is required
		return;
	}
	strError += this->setRole(domElem.attribute("Role"));
}
