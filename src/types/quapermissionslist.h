#ifndef QUAPERMISSIONSLIST_H
#define QUAPERMISSIONSLIST_H

#include <QUaFolderObject>

#include <QDomDocument>
#include <QDomElement>

class QUaPermissions;

class QUaPermissionsList : public QUaFolderObject
{
	friend class QUaPermissions;
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaPermissionsList(QUaServer *server);

	// UA methods

	Q_INVOKABLE QString addPermissions(QString strId);

	Q_INVOKABLE void clear();

	// C++ API

	QList<QUaPermissions*> permissions();

	// XML import / export
	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

signals:

public slots:

private:

};

#endif // QUAPERMISSIONSLIST_H