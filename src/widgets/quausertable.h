#ifndef QUAUSERTABLE_H
#define QUAUSERTABLE_H

#include <QWidget>
#include <QStandardItemModel>
#include <QUaAcCommonWidgets>

namespace Ui {
class QUaUserTable;
}

class QUaAccessControl;
class QUaUser;
class QUaAcCommonDialog;

class QUaUserTable : public QWidget
{
    Q_OBJECT
	// expose to style
	Q_PROPERTY(QIcon iconAdd        READ iconAdd        WRITE setIconAdd   )
	Q_PROPERTY(QIcon iconEdit       READ iconEdit       WRITE setIconEdit  )
	Q_PROPERTY(QIcon iconDelete     READ iconDelete     WRITE setIconDelete)
	Q_PROPERTY(QIcon iconClear      READ iconClear      WRITE setIconClear )

public:
    explicit QUaUserTable(QWidget *parent = nullptr);
    ~QUaUserTable();

	bool isAddVisible() const;
	void setAddVisible(const bool &isVisible);

	QUaAccessControl * accessControl() const;
	void setAccessControl(QUaAccessControl * ac);

	// stylesheet

	QIcon iconAdd() const;
	void  setIconAdd(const QIcon& icon);

	QIcon iconEdit() const;
	void  setIconEdit(const QIcon& icon);

	QIcon iconDelete() const;
	void  setIconDelete(const QIcon& icon);

	QIcon iconClear() const;
	void  setIconClear(const QIcon& icon);

	QByteArray headerState() const;
	void setHeaderState(const QByteArray& state);

	// table headers
	enum class Headers
	{
		Name      = 0,
		Role      = 1,
		Invalid   = 2
	};
	Q_ENUM(Headers)

signals:
	void userSelectionChanged(QUaUser * userPrev, QUaUser * userCurr);
	void userDoubleClicked(QUaUser* user);
	void userEditClicked(QUaUser* user);
	void showRolesClicked();

public slots:
	void on_loggedUserChanged(QUaUser * user);

private slots:
    void on_pushButtonAdd_clicked();

private:
    Ui::QUaUserTable *ui;
	QUaAccessControl     * m_ac;
	QStandardItemModel     m_modelUsers;
	QUaAcLambdaFilterProxy m_proxyUsers;
	QUaUser              * m_loggedUser;

	QIcon m_iconAdd;
	QIcon m_iconEdit;
	QIcon m_iconDelete;
	QIcon m_iconClear;

	void setupTableContextMenu();

	void showNewUserDialog(QUaAcCommonDialog &dialog);
	QStandardItem *  handleUserAdded(QUaUser * user);

	// static values
	static int PointerRole;
};

#endif // QUAUSERTABLE_H
