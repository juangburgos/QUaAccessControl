#include "quapermissions.h"

QUaPermissions::QUaPermissions(QUaServer *server)
	: QUaBaseObject(server)
{

}

void QUaPermissions::remove()
{
	this->deleteLater();
}
