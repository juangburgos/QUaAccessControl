#ifndef QUAROLELIST_H
#define QUAROLELIST_H

#include <QUaFolderObjectProtected>

#include <QDomDocument>
#include <QDomElement>

class QUaRole;

class QUaRoleList : public QUaFolderObjectProtected
{
	friend class QUaRole;
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaRoleList(QUaServer *server);

	// UA methods

	Q_INVOKABLE QString addRole(QString strName);

	Q_INVOKABLE void    clear();

	Q_INVOKABLE QString xmlConfig();

	Q_INVOKABLE QString setXmlConfig(QString strXmlConfig);

	// C++ API

	QList<QUaRole*> roles() const;

	QUaRole * role(const QString &strName) const;

	// XML import / export
	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

signals:

public slots:

private:

};

#endif // QUAROLELIST_H