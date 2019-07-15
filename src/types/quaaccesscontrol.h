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

	// UA variables

	QUaUserList        * users() const;

	QUaRoleList        * roles() const;

	QUaPermissionsList * permissions() const;

	// C++ API

	bool       hasRootUser() const;
	QUaUser  * rootUser() const;
	void       setRootUser(QUaUser * rootUser);
	void       clearRootUser();

	// XML import / export
	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

	// static
	static QUaReference HasRootUserRefType;

signals:
	void rootUserChanged(QUaUser * rootUser);

public slots:
};

#endif // QUAACCESSCONTROL_H