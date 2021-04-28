#ifndef QUAROLETABLE_H
#define QUAROLETABLE_H

#include <QWidget>
#include <QStandardItemModel>
#include <QUaAcCommonWidgets>

namespace Ui {
class QUaRoleTable;
}

class QUaAccessControl;
class QUaRole;
class QUaUser;
class QUaAcCommonDialog;

class QUaRoleTable : public QWidget
{
    Q_OBJECT
	// expose to style
	Q_PROPERTY(QIcon iconAdd        READ iconAdd        WRITE setIconAdd   )
	Q_PROPERTY(QIcon iconEdit       READ iconEdit       WRITE setIconEdit  )
	Q_PROPERTY(QIcon iconDelete     READ iconDelete     WRITE setIconDelete)
	Q_PROPERTY(QIcon iconClear      READ iconClear      WRITE setIconClear )

public:
    explicit QUaRoleTable(QWidget *parent = nullptr);
    ~QUaRoleTable();

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
		Invalid   = 1
	};
	Q_ENUM(Headers)

signals:
	void roleSelectionChanged(QUaRole * rolePrev, QUaRole * roleCurr);
	void roleDoubleClicked(QUaRole* role);
	void roleEditClicked(QUaRole* role);

public slots:
	void on_loggedUserChanged(QUaUser * user);

private slots:
    void on_pushButtonAdd_clicked();

private:
    Ui::QUaRoleTable *ui;
	QUaAccessControl     * m_ac;
	QStandardItemModel     m_modelRoles;
	QUaAcLambdaFilterProxy m_proxyRoles;
	QUaUser              * m_loggedUser;

	QIcon m_iconAdd;
	QIcon m_iconEdit;
	QIcon m_iconDelete;
	QIcon m_iconClear;

	void setupTableContextMenu();

	void showNewRoleDialog(QUaAcCommonDialog &dialog);
	QStandardItem *  handleRoleAdded(QUaRole * role);

	// static values
	static int PointerRole;
};

#endif // QUAROLETABLE_H
