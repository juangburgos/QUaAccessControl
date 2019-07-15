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

	QString addUser(const QString &strName, const QByteArray &bytaHash);

	QList<QUaUser*> users() const;

	QUaUser * user(const QString &strName) const;

	QUaAccessControl * accessControl() const;

	// XML import / export
	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElementInstantiate(QDomElement  & domElem, QString &strError);
	void        fromDomElementConfigure  (QDomElement  & domElem, QString &strError);

signals:
	void userAdded  (QUaUser * user);
	void userRemoved(QUaUser * user);

private slots:
	void on_childAdded(QUaNode * node);

private:
	bool isUserNameValid(QString &strName, QString &strError);
};

#endif // QUAUSERLIST_H