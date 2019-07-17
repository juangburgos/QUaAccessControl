#ifndef QUAROLEWIDGETEDIT_H
#define QUAROLEWIDGETEDIT_H

#include <QWidget>

#include <QStandardItemModel>
#include <QUaAcCommonWidgets>

namespace Ui {
class QUaRoleWidgetEdit;
}

class QUaUser;

class QUaRoleWidgetEdit : public QWidget
{
    Q_OBJECT

public:
    explicit QUaRoleWidgetEdit(QWidget *parent = nullptr);
    ~QUaRoleWidgetEdit();

	bool isRoleNameReadOnly() const;
	void setRoleNameReadOnly(const bool &readOnly);

	bool areActionsVisible() const;
	void setActionsVisible(const bool &isVisible);

	bool isUserListVisible() const;
	void setUserListVisible(const bool &isVisible);

	QString roleName() const;
	void    setRoleName(const QString &strRoleName);

	QStringList users() const;
	void        setUsers(const QStringList &listUsers);
	void        addUser(const QString &strUserName);
	void        removeUser(const QString &strUserName);

	// table headers
	enum class Headers
	{
		Name    = 0,
		Invalid = 1
	};
	Q_ENUM(Headers)

signals:
	void deleteClicked();

public slots:
	void on_loggedUserChanged(QUaUser * user);

private:
    Ui::QUaRoleWidgetEdit *ui;

	QStandardItemModel     m_modelUsers;
	QUaAcLambdaFilterProxy m_proxyUsers;

	QUaUser              * m_loggedUser;
};

#endif // QUAROLEWIDGETEDIT_H
