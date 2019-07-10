#ifndef QUABASEOBJECTPROTECTED_H
#define QUABASEOBJECTPROTECTED_H

#include <QUaBaseObject>

class QUaBaseObjectProtected : public QUaBaseObject
{
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaBaseObjectProtected(QUaServer *server);

signals:

public slots:
};

#endif // QUABASEOBJECTPROTECTED_H