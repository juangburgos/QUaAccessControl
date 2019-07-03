#ifndef QUAUSER_H
#define QUAUSER_H

#include <QUaBaseObject>

#include <QDomDocument>
#include <QDomElement>

class QUaUserList;

class QUaUser : public QUaBaseObject
{
	friend class QUaUserList;
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaUser(QUaServer *server);

	// UA methods

	Q_INVOKABLE void remove();

signals:

public slots:
};

#endif // QUAUSER_H