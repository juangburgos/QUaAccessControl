#include "quarole.h"

#include <QUaUser>

QUaRole::QUaRole(QUaServer *server)
	: QUaBaseObjectProtected(server)
{
	QObject::connect(this, &QUaNode::referenceAdded  , this, &QUaRole::on_referenceAdded);
	QObject::connect(this, &QUaNode::referenceRemoved, this, &QUaRole::on_referenceRemoved);
}

void QUaRole::remove()
{
	this->deleteLater();
}

QString QUaRole::getName() const
{
	return this->browseName();
}

QList<QUaUser*> QUaRole::users() const
{
	return this->findReferences<QUaUser>(QUaUser::UserHasRoleRefType, false);
}

bool QUaRole::hasUser(QUaUser * user) const
{
	return this->users().contains(user);
}

QDomElement QUaRole::toDomElement(QDomDocument & domDoc) const
{
	// add element
	QDomElement elem = domDoc.createElement(QUaRole::staticMetaObject.className());
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
	// nothing else to add!
}

void QUaRole::on_referenceAdded(const QUaReference & ref, QUaNode * nodeTarget, const bool & isForward)
{
	if (ref != QUaUser::UserHasRoleRefType)
	{
		return;
	}
	Q_ASSERT(!isForward);
	auto user = dynamic_cast<QUaUser*>(nodeTarget);
	if (!user)
	{
		return;
	}
	// only emit if user still valid
	emit this->userAdded(user);
}

void QUaRole::on_referenceRemoved(const QUaReference & ref, QUaNode * nodeTarget, const bool & isForward)
{
	if (ref != QUaUser::UserHasRoleRefType)
	{
		return;
	}
	Q_ASSERT(!isForward);
	auto user = dynamic_cast<QUaUser*>(nodeTarget);
	if (!user)
	{
		return;
	}
	// only emit if user still valid
	emit this->userRemoved(user);
}
