#ifndef QUAACCESSCONTROL_H
#define QUAACCESSCONTROL_H

#include <QUaFolderObject>

#include <QUaUserList>
#include <QUaRoleList>
#include <QUaPermissionsList>

class QUaAccessControl : public QUaFolderObject
{
    Q_OBJECT

	Q_PROPERTY(QUaUserList        * Users       READ users        )
	Q_PROPERTY(QUaRoleList        * Roles       READ roles        )
	Q_PROPERTY(QUaPermissionsList * Permissions READ permissions  )

public:
	Q_INVOKABLE explicit QUaAccessControl(QUaServer *server);

	// UA variables

	QUaUserList        * users() const;

	QUaRoleList        * roles() const;

	QUaPermissionsList * permissions() const;


signals:

public slots:
};

#endif // QUAACCESSCONTROL_H