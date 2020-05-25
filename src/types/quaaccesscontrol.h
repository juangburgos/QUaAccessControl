#ifndef QUAACCESSCONTROL_H
#define QUAACCESSCONTROL_H

#include <QUaFolderObjectProtected>

#include <QUaUserList>
#include <QUaRoleList>
#include <QUaPermissionsList>

class QUaAccessControl : public QUaFolderObjectProtected
{
    Q_OBJECT

	Q_PROPERTY(QUaUserList        * Users       READ users        )
	Q_PROPERTY(QUaRoleList        * Roles       READ roles        )
	Q_PROPERTY(QUaPermissionsList * Permissions READ permissions  )

public:
	Q_INVOKABLE explicit QUaAccessControl(QUaServer *server);

	// UA methods

	Q_INVOKABLE QString xmlConfig();

	Q_INVOKABLE QString setXmlConfig(QString strXmlConfig);

	Q_INVOKABLE void clear();

	// UA variables

	QUaUserList        * users() const;

	QUaRoleList        * roles() const;

	QUaPermissionsList * permissions() const;

	// C++ API

	bool       hasRootUser() const;
	QUaUser  * rootUser() const;
	void       setRootUser(QUaUser * rootUser);
	void       clearRootUser();
	void       clearInmediatly();

	// XML import / export
	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QQueue<QUaLog>& errorLogs);

	// static
	static QUaReferenceType HasRootUserRefType;

signals:
	void rootUserChanged(QUaUser * rootUser);

public slots:
};

#endif // QUAACCESSCONTROL_H