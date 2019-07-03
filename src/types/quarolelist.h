#ifndef QUAROLELIST_H
#define QUAROLELIST_H

#include <QUaFolderObject>

#include <QDomDocument>
#include <QDomElement>

class QUaRole;

class QUaRoleList : public QUaFolderObject
{
	friend class QUaRole;
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaRoleList(QUaServer *server);

	// UA methods

	Q_INVOKABLE QString addRole(QString strName);

	Q_INVOKABLE void clear();

	// C++ API

	QList<QUaRole*> roles();

	// XML import / export
	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

signals:

public slots:

private:

};

#endif // QUAROLELIST_H