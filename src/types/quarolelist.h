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
	void        fromDomElementInstantiate(QDomElement  & domElem, QQueue<QUaLog>& errorLogs);
	void        fromDomElementConfigure  (QDomElement  & domElem, QQueue<QUaLog>& errorLogs);

signals:
	void roleAdded  (QUaRole * role);
	void roleRemoved(QUaRole * role);

private slots:
	void on_childAdded(QUaNode * node);

private:

};

#endif // QUAROLELIST_H