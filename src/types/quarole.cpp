#include "quarole.h"

QUaRole::QUaRole(QUaServer *server)
	: QUaBaseObject(server)
{

}

void QUaRole::remove()
{
	this->deleteLater();
}
