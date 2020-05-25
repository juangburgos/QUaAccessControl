#ifndef QUAPERMISSIONSLIST_H
#define QUAPERMISSIONSLIST_H

#include <QUaFolderObjectProtected>

#include <QDomDocument>
#include <QDomElement>

class QUaPermissions;
class QUaAccessControl;

class QUaPermissionsList : public QUaFolderObjectProtected
{
	friend class QUaPermissions;
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaPermissionsList(QUaServer *server);

	// UA methods

	Q_INVOKABLE QString addPermissions(QString strId);

	Q_INVOKABLE void clear();

	Q_INVOKABLE QString xmlConfig();

	Q_INVOKABLE QString setXmlConfig(QString strXmlConfig);

	// C++ API

	QList<QUaPermissions*> permissionsList() const;

	QUaPermissions * permission(const QString &strId) const;

	QUaAccessControl * accessControl() const;

	// XML import / export
	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElementInstantiate(QDomElement  & domElem, QQueue<QUaLog>& errorLogs);
	void        fromDomElementConfigure  (QDomElement  & domElem, QQueue<QUaLog>& errorLogs);

signals:
	void permissionsAdded  (QUaPermissions * permissions);
	void permissionsRemoved(QUaPermissions * permissions);

private slots:
	void on_childAdded(QUaNode * node);

private:

};

#endif // QUAPERMISSIONSLIST_H