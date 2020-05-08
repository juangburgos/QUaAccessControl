#include "quarole.h"

#include <QUaRoleList>
#include <QUaUser>
#include <QUaPermissions>

QUaRole::QUaRole(QUaServer *server)
	: QUaBaseObjectProtected(server)
{
	QObject::connect(this, &QUaNode::referenceAdded  , this, &QUaRole::on_referenceAdded);
	QObject::connect(this, &QUaNode::referenceRemoved, this, &QUaRole::on_referenceRemoved);
}

void QUaRole::remove()
{
	this->deleteLater();
	// NOTE : destroyed signal is too late
	emit this->list()->roleRemoved(this);
}

QString QUaRole::getName() const
{
	return this->browseName().name();
}

QList<QUaUser*> QUaRole::users() const
{
	return this->findReferences<QUaUser>(QUaUser::UserHasRoleRefType, false);
}

bool QUaRole::hasUser(QUaUser * user) const
{
	return this->users().contains(user);
}

QUaRoleList * QUaRole::list() const
{
	return dynamic_cast<QUaRoleList*>(this->parent());
}

QDomElement QUaRole::toDomElement(QDomDocument & domDoc) const
{
	// add element
	QDomElement elem = domDoc.createElement(QUaRole::staticMetaObject.className());
	// set parmissions if any
	if (this->hasPermissionsObject())
	{
		elem.setAttribute("Permissions", this->permissionsObject()->nodeId());
	}
	// set all attributes
	elem.setAttribute("Name", this->getName());
	// return element
	return elem;
}

void QUaRole::fromDomElement(QDomElement & domElem, QString & strError)
{
	Q_UNUSED(strError);
	// NOTE : at this point name must already be set
	Q_ASSERT(this->getName().compare(domElem.attribute("Name"), Qt::CaseSensitive) == 0);
	// load permissions if any
	if (domElem.hasAttribute("Permissions") && !domElem.attribute("Permissions").isEmpty())
	{
		strError += this->setPermissions(domElem.attribute("Permissions"));
	}
}

void QUaRole::on_referenceAdded(const QUaReferenceType& ref, QUaNode * nodeTarget, const bool & isForward)
{
	if (ref != QUaUser::UserHasRoleRefType)
	{
		return;
	}
	Q_ASSERT(!isForward);
	auto user = dynamic_cast<QUaUser*>(nodeTarget);
	//Q_CHECK_PTR(user); happens on shutdown
	if (!user)
	{
		return;
	}
	// only emit if user still valid
	emit this->userAdded(user);
}

void QUaRole::on_referenceRemoved(const QUaReferenceType& ref, QUaNode * nodeTarget, const bool & isForward)
{
	if (ref != QUaUser::UserHasRoleRefType)
	{
		return;
	}
	Q_ASSERT(!isForward);
	auto user = dynamic_cast<QUaUser*>(nodeTarget);
	//Q_CHECK_PTR(user); happens on shutdown
	if (!user)
	{
		return;
	}
	// only emit if user still valid
	emit this->userRemoved(user);
}
