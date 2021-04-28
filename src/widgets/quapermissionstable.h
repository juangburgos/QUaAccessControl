#ifndef QUAPERMISSIONSTABLE_H
#define QUAPERMISSIONSTABLE_H

#include <QWidget>
#include <QStandardItemModel>
#include <QUaAcCommonWidgets>

namespace Ui {
class QUaPermissionsTable;
}

class QUaAccessControl;
class QUaRole;
class QUaUser;
class QUaPermissions;
class QUaAcCommonDialog;

class QUaPermissionsTable : public QWidget
{
    Q_OBJECT
	// expose to style
	Q_PROPERTY(QIcon iconAdd        READ iconAdd        WRITE setIconAdd   )
	Q_PROPERTY(QIcon iconEdit       READ iconEdit       WRITE setIconEdit  )
	Q_PROPERTY(QIcon iconDelete     READ iconDelete     WRITE setIconDelete)
	Q_PROPERTY(QIcon iconClear      READ iconClear      WRITE setIconClear )

public:
    explicit QUaPermissionsTable(QWidget *parent = nullptr);
    ~QUaPermissionsTable();

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
		Id      = 0,
		Invalid = 1
	};
	Q_ENUM(Headers)

signals:
	void permissionsSelectionChanged(QUaPermissions * permsPrev, QUaPermissions * permsCurr);
	void permissionsDoubleClicked(QUaPermissions* perms);
	void permissionsEditClicked(QUaPermissions* perms);

public slots:
	void on_loggedUserChanged(QUaUser * user);

private slots:
    void on_pushButtonAdd_clicked();

private:
    Ui::QUaPermissionsTable *ui;
	QUaAccessControl     * m_ac;
	QStandardItemModel     m_modelPerms;
	QUaAcLambdaFilterProxy m_proxyPerms;
	QUaUser              * m_loggedUser;

	QIcon m_iconAdd;
	QIcon m_iconEdit;
	QIcon m_iconDelete;
	QIcon m_iconClear;

	void setupTableContextMenu();

	void showNewPermissionsDialog(QUaAcCommonDialog &dialog);
	QStandardItem *  handlePermssionsAdded(QUaPermissions * perms);

	// static values
	static int PointerRole;
};

#endif // QUAPERMISSIONSTABLE_H
