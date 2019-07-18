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

public:
    explicit QUaPermissionsTable(QWidget *parent = nullptr);
    ~QUaPermissionsTable();

	bool isAddVisible() const;
	void setAddVisible(const bool &isVisible);

	QUaAccessControl * accessControl() const;
	void setAccessControl(QUaAccessControl * ac);

	// table headers
	enum class Headers
	{
		Id      = 0,
		Invalid = 1
	};
	Q_ENUM(Headers)

signals:
	void permissionsSelectionChanged(QUaPermissions * permsPrev, QUaPermissions * permsCurr);

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

	void showNewPermissionsDialog(QUaAcCommonDialog &dialog);
	QStandardItem *  handlePermssionsAdded(QUaPermissions * perms);

	// static values
	static int PointerRole;
};

#endif // QUAPERMISSIONSTABLE_H
