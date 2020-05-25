#ifndef QUAUSERLIST_H
#define QUAUSERLIST_H

#include <QUaFolderObjectProtected>

#include <QDomDocument>
#include <QDomElement>

class QUaUser;
class QUaAccessControl;

class QUaUserList : public QUaFolderObjectProtected
{
	friend class QUaUser;
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaUserList(QUaServer *server);

	// UA methods

	Q_INVOKABLE QString addUser(QString strName, QString strPassword);

	Q_INVOKABLE void clear();

	Q_INVOKABLE QString xmlConfig();

	Q_INVOKABLE QString setXmlConfig(QString strXmlConfig);

	// C++ API

	QQueue<QUaLog> addUser(const QString &strName, const QByteArray &bytaHash);

	QList<QUaUser*> users() const;

	QUaUser * user(const QString &strName) const;

	QUaAccessControl * accessControl() const;

	// XML import / export
	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElementInstantiate(QDomElement  & domElem, QQueue<QUaLog>& errorLogs);
	void        fromDomElementConfigure  (QDomElement  & domElem, QQueue<QUaLog>& errorLogs);

signals:
	void userAdded  (QUaUser * user);
	void userRemoved(QUaUser * user);

private slots:
	void on_childAdded(QUaNode * node);

private:
	bool isUserNameValid(QString &strName, QQueue<QUaLog>& errorLogs);
};

#endif // QUAUSERLIST_H