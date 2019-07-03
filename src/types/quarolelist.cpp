#include "quarolelist.h"

// NOTE : had to add this header because the actual implementation of QUaBaseObject::addChild is in here
//        and was getting "lnk2019 unresolved external symbol template function" without it
#include <QUaServer>
#include <QUaRole>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

QUaRoleList::QUaRoleList(QUaServer *server)
	: QUaFolderObject(server)
{

}

QString QUaRoleList::addRole(QString strName)
{
	// TODO
	return QString();
}

void QUaRoleList::clear()
{
	// TODO
	return void();
}

QList<QUaRole*> QUaRoleList::roles()
{
	return this->browseChildren<QUaRole>();
}
