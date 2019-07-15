#ifndef QUAUSERWIDGETEDIT_H
#define QUAUSERWIDGETEDIT_H

#include <QWidget>

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

	bool isRoleVisible() const;
	void setRoleVisible(const bool &isVisible);

	bool isHashVisible() const;
	void setHashVisible(const bool &isVisible);

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
};

#endif // QUAUSERWIDGETEDIT_H
