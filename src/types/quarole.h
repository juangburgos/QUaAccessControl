#ifndef QUAROLE_H
#define QUAROLE_H

#include <QUaBaseObject>

#include <QDomDocument>
#include <QDomElement>

class QUaRoleList;

class QUaUser;

class QUaRole : public QUaBaseObject
{
	friend class QUaRoleList;
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaRole(QUaServer *server);

	// UA methods

	Q_INVOKABLE void remove();

	// C++ API

	QString         getName() const;

	QList<QUaUser*> users() const;

	bool            hasUser(QUaUser * user) const;

	// XML import / export
	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

signals:
	void userAdded  (QUaUser * user);
	void userRemoved(QUaUser * user);

public slots:

private slots:
	void on_referenceAdded  (const QUaReference & ref, QUaNode * nodeTarget, const bool &isForward);
	void on_referenceRemoved(const QUaReference & ref, QUaNode * nodeTarget, const bool &isForward);
};

#endif // QUAROLE_H