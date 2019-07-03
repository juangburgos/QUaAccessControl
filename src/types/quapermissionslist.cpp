#include "quapermissionslist.h"

// NOTE : had to add this header because the actual implementation of QUaBaseObject::addChild is in here
//        and was getting "lnk2019 unresolved external symbol template function" without it
#include <QUaServer>
#include <QUaPermissions>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

QUaPermissionsList::QUaPermissionsList(QUaServer *server)
	: QUaFolderObject(server)
{

}

QString QUaPermissionsList::addPermissions(QString strId)
{
	// TODO
	return QString();
}

void QUaPermissionsList::clear()
{
	// TODO
	return void();
}

QList<QUaPermissions*> QUaPermissionsList::permissions()
{
	return this->browseChildren<QUaPermissions>();
}
