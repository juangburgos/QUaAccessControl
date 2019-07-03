#include "quarole.h"

QUaRole::QUaRole(QUaServer *server)
	: QUaBaseObject(server)
{

}

void QUaRole::remove()
{
	this->deleteLater();
}

QString QUaRole::getName() const
{
	return this->browseName();
}
