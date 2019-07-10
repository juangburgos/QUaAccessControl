#ifndef QUAPERMISSIONS_H
#define QUAPERMISSIONS_H

#include <QUaBaseObjectProtected>

#include <QDomDocument>
#include <QDomElement>

class QUaRole;
class QUaUser;
class QUaPermissionsList;

class QUaPermissions : public QUaBaseObjectProtected
{
	friend class QUaPermissionsList;
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaPermissions(QUaServer *server);

	// UA methods

	Q_INVOKABLE void remove();

	Q_INVOKABLE QString addRoleCanRead(QList<QString> strRolePath);

	Q_INVOKABLE QString removeRoleCanRead(QList<QString> strRolePath);

	Q_INVOKABLE QString addRoleCanWrite(QList<QString> strRolePath);

	Q_INVOKABLE QString removeRoleCanWrite(QList<QString> strRolePath);

	// C++ API

	QString getId() const;

	void addRoleCanRead    (QUaRole * role);
	void removeRoleCanRead (QUaRole * role);
	void addRoleCanWrite   (QUaRole * role);
	void removeRoleCanWrite(QUaRole * role);

	QList<QUaRole*> rolesCanRead () const;
	QList<QUaRole*> rolesCanWrite() const;
	QList<QUaUser*> usersCanRead () const;
	QList<QUaUser*> usersCanWrite() const;

	bool canRoleRead (QUaRole * role) const;
	bool canRoleWrite(QUaRole * role) const;
	bool canUserRead (QUaUser * user) const;
	bool canUserWrite(QUaUser * user) const;

	bool canRoleRead (const QString strRoleName) const;
	bool canRoleWrite(const QString strRoleName) const;
	bool canUserRead (const QString strUserName) const;
	bool canUserWrite(const QString strUserName) const;

	QUaPermissionsList * list() const;

	// XML import / export
	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

	// static
	static QUaReference IsReadableByRefType;
	static QUaReference IsWritableByRefType;
	static QUaReference HasPermissionsRefType;

signals:
	void canReadRoleAdded   (QUaRole * role);
	void canReadRoleRemoved (QUaRole * role);
	void canWriteRoleAdded  (QUaRole * role);
	void canWriteRoleRemoved(QUaRole * role);

public slots:

private:
	QUaRole * findRole(const QList<QString> &strRolePath, QString &strError) const;

};

#endif // QUAPERMISSIONS_H