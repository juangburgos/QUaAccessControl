#ifndef QUAROLE_H
#define QUAROLE_H

#include <QUaBaseObject>

#include <QDomDocument>
#include <QDomElement>

class QUaRoleList;

class QUaRole : public QUaBaseObject
{
	friend class QUaRoleList;
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaRole(QUaServer *server);

	// UA methods

	Q_INVOKABLE void remove();

signals:

public slots:
};

#endif // QUAROLE_H