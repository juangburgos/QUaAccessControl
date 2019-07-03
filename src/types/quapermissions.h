#ifndef QUAPERMISSIONS_H
#define QUAPERMISSIONS_H

#include <QUaBaseObject>

#include <QDomDocument>
#include <QDomElement>

class QUaPermissionsList;

class QUaPermissions : public QUaBaseObject
{
	friend class QUaPermissionsList;
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaPermissions(QUaServer *server);

	// UA methods

	Q_INVOKABLE void remove();

signals:

public slots:
};

#endif // QUAPERMISSIONS_H