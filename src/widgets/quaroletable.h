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

public:
    explicit QUaRoleTable(QWidget *parent = nullptr);
    ~QUaRoleTable();

	bool isAddVisible() const;
	void setAddVisible(const bool &isVisible);

	QUaAccessControl * accessControl() const;
	void setAccessControl(QUaAccessControl * ac);

	// table headers
	enum class Headers
	{
		Name      = 0,
		Invalid   = 1
	};
	Q_ENUM(Headers)

signals:
	void roleSelectionChanged(QUaRole * rolePrev, QUaRole * roleCurr);

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

	void showNewRoleDialog(QUaAcCommonDialog &dialog);
	QStandardItem *  handleRoleAdded(QUaRole * role);

	// static values
	static int PointerRole;
};

#endif // QUAROLETABLE_H
