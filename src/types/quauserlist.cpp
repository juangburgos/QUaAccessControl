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
	// TODO
	return QString();
}

void QUaUserList::clear()
{
	// TODO
	return void();
}

QList<QUaUser*> QUaUserList::users()
{
	return this->browseChildren<QUaUser>();
}
