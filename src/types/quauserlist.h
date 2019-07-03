#ifndef QUAUSERLIST_H
#define QUAUSERLIST_H

#include <QUaFolderObject>

#include <QDomDocument>
#include <QDomElement>

class QUaUser;

class QUaUserList : public QUaFolderObject
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

	QList<QUaUser*> users() const;

	// XML import / export
	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

signals:

public slots:

private:

};

#endif // QUAUSERLIST_H