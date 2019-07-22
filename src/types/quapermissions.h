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

	Q_INVOKABLE QString addRoleCanRead(QString strRoleNodeId);

	Q_INVOKABLE QString removeRoleCanRead(QString strRoleNodeId);

	Q_INVOKABLE QString addRoleCanWrite(QString strRoleNodeId);

	Q_INVOKABLE QString removeRoleCanWrite(QString strRoleNodeId);

	Q_INVOKABLE QString addUserCanRead(QString strUserNodeId);

	Q_INVOKABLE QString removeUserCanRead(QString strUserNodeId);

	Q_INVOKABLE QString addUserCanWrite(QString strUserNodeId);

	Q_INVOKABLE QString removeUserCanWrite(QString strUserNodeId);

	// C++ API

	QString getId() const;

	void addRoleCanRead    (QUaRole * role);
	void removeRoleCanRead (QUaRole * role);
	void addRoleCanWrite   (QUaRole * role);
	void removeRoleCanWrite(QUaRole * role);

	void addUserCanRead    (QUaUser * user);
	void removeUserCanRead (QUaUser * user);
	void addUserCanWrite   (QUaUser * user);
	void removeUserCanWrite(QUaUser * user);

	QList<QUaRole*> rolesCanRead () const;
	QList<QUaRole*> rolesCanWrite() const;
	QList<QUaUser*> usersCanRead () const;
	QList<QUaUser*> usersCanWrite() const;

	// directly == not due to role permissions
	QList<QUaUser*> usersCanReadDirectly () const;
	QList<QUaUser*> usersCanWriteDirectly() const;

	bool canRoleRead (QUaRole * role) const;
	bool canRoleWrite(QUaRole * role) const;
	bool canUserRead (QUaUser * user) const;
	bool canUserWrite(QUaUser * user) const;

	// directly == not due to role permissions
	bool canUserReadDirectly (QUaUser * user) const;
	bool canUserWriteDirectly(QUaUser * user) const;

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
	void canReadUserAdded   (QUaUser * user);
	void canReadUserRemoved (QUaUser * user);
	void canWriteUserAdded  (QUaUser * user);
	void canWriteUserRemoved(QUaUser * user);

public slots:

private:
	QUaRole * findRole(const QString &strRoleNodeId, QString &strError) const;
	QUaUser * findUser(const QString &strUserNodeId, QString &strError) const;

};

#endif // QUAPERMISSIONS_H