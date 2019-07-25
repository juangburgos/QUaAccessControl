#ifndef QUAPERMISSIONSWIDGETEDIT_H
#define QUAPERMISSIONSWIDGETEDIT_H

#include <QWidget>

#include <QStandardItemModel>
#include <QSortFilterProxyModel>

namespace Ui {
class QUaPermissionsWidgetEdit;
}

struct QUaRoleAccess
{
	bool canRead;
	bool canWrite;
};
typedef QMap<QString, QUaRoleAccess> QUaRoleAccessMap;
typedef QMapIterator<QString, QUaRoleAccess> QUaRoleAccessIter;

struct QUaUserAccess
{
	bool canUserRead;
	bool canUserWrite;
	bool canRoleRead;  // just refex dedundant info from QUaRoleAccess
	bool canRoleWrite; // just refex dedundant info from QUaRoleAccess
};
typedef QMap<QString, QUaUserAccess> QUaUserAccessMap;
typedef QMapIterator<QString, QUaUserAccess> QUaUserAccessIter;

class QUaPermissionsWidgetEdit : public QWidget
{
    Q_OBJECT

public:
    explicit QUaPermissionsWidgetEdit(QWidget *parent = nullptr);
    ~QUaPermissionsWidgetEdit();

	bool isIdReadOnly() const;
	void setIdReadOnly(const bool &readOnly);

	bool isIdVisible() const;
	void setIdVisible(const bool &isVisible);

	bool isAccessReadOnly() const;
	void setAccessReadOnly(const bool &readOnly);

	bool areAccessVisible() const;
	void setAccessVisible(const bool &isVisible);

	bool areActionsVisible() const;
	void setActionsVisible(const bool &isVisible);

	QString id() const;
	void    setId(const QString &strId);

	QUaRoleAccessMap roleAccessMap() const;
	void setRoleAccessMap(const QUaRoleAccessMap &roleMap); //completely overwrites
	void updateRoleAccess(const QString &strRoleName, const QUaRoleAccess &roleAccess);

	QUaUserAccessMap userAccessMap() const;
	void setUserAccessMap(const QUaUserAccessMap &userMap); //completely overwrites
	void updateUserAccess(const QString &strUserName, const QUaUserAccess &userAccess);

	// role table headers
	enum class RoleHeaders
	{
		Name    = 0,
		Read    = 1,
		Write   = 2,
		Invalid = 3
	};
	Q_ENUM(RoleHeaders)

	// user table headers
	enum class UserHeaders
	{
		Name      = 0,
		UserRead  = 1,
		RoleRead  = 2,
		UserWrite = 3,
		RoleWrite = 4,
		Invalid   = 5
	};
	Q_ENUM(UserHeaders)

signals:
	void deleteClicked();
	void applyClicked();

private:
    Ui::QUaPermissionsWidgetEdit *ui;
	bool m_accessReadOnly;

	QStandardItemModel    m_modelUsers;
	QSortFilterProxyModel m_proxyUsers;
	QStandardItemModel    m_modelRoles;
	QSortFilterProxyModel m_proxyRoles;
};

#endif // QUAPERMISSIONSWIDGETEDIT_H
