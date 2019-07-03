#ifndef QUAUSER_H
#define QUAUSER_H

#include <QUaBaseObject>
#include <QUaProperty>

#include <QDomDocument>
#include <QDomElement>

class QUaUserList;
class QUaRole;

class QUaUser : public QUaBaseObject
{
	friend class QUaUserList;
    Q_OBJECT

	// NOTE : user name is already browse name, no need to put it as a prop

	// UA properties
	Q_PROPERTY(QUaProperty * Hash READ hash)

public:
	Q_INVOKABLE explicit QUaUser(QUaServer *server);

	// UA properties

	QUaProperty * hash() const;

	// UA methods

	Q_INVOKABLE void    remove();
	// overwrite password, does not care about old password
	Q_INVOKABLE QString setPassword(QString strPassword);
	// need password to set role because server does not store passwords 
	// (no way to recalculate the hash that depends on password and role name)
	Q_INVOKABLE QString setRole(QString strPassword, QList<QString> strRolePath);
	// same as above
	Q_INVOKABLE QString clearRole(QString strPassword);

	// C++ API

	QString    getName() const;

	QByteArray getHash() const;
	void       setHash(const QByteArray &hash);

	bool       hasRole() const;

	QUaRole  * getRole() const;

	bool       isPasswordValid(const QString &strPassword) const;

	// XML import / export
	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

	// static
	static QUaReference UserHasRoleRefType;

signals:
	// C++ API
	void hashChanged(const QByteArray &hash);
	void roleChanged(QUaRole * role);
};

#endif // QUAUSER_H