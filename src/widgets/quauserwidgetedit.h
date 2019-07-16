#ifndef QUAUSERWIDGETEDIT_H
#define QUAUSERWIDGETEDIT_H

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QCompleter>

namespace Ui {
class QUaUserWidgetEdit;
}

class QUaRoleList;
class QUaRole;

class QUaUserWidgetEdit : public QWidget
{
    Q_OBJECT

public:
    explicit QUaUserWidgetEdit(QWidget *parent = nullptr);
    ~QUaUserWidgetEdit();

	bool isUserNameReadOnly() const;
	void setUserNameReadOnly(const bool &readOnly);

	bool isRoleVisible() const;
	void setRoleVisible(const bool &isVisible);

	bool isRoleReadOnly() const;
	void setRoleReadOnly(const bool &readOnly);

	bool isHashVisible() const;
	void setHashVisible(const bool &isVisible);

	bool isPasswordVisible() const;
	void setPasswordVisible(const bool &isVisible);

	void setRoleList(const QUaRoleList * listRoles);

	QString userName() const;
	void    setUserName(const QString &strUserName);

	QUaRole * role() const;
	void      setRole(const QUaRole * role);

	QString password() const;
	void    setPassword(const QString &strPassword);

	QString hash() const;
	void    setHash(const QString &strHexHash);

private:
    Ui::QUaUserWidgetEdit *ui;

	QStandardItemModel    m_modelCombo;
	QSortFilterProxyModel m_proxyCombo;

	QList<QMetaObject::Connection> m_connections;

	static int PointerRole;
};

#endif // QUAUSERWIDGETEDIT_H
