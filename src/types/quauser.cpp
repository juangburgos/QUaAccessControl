#include "quauser.h"

QUaUser::QUaUser(QUaServer *server)
	: QUaBaseObject(server)
{

}

void QUaUser::remove()
{
	this->deleteLater();
}
