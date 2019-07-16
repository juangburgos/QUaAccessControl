#ifndef QUAUSERTABLE_H
#define QUAUSERTABLE_H

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

namespace Ui {
class QUaUserTable;
}

class QUaAccessControl;
class QUaUser;
class QUaAcCommonDialog;

class QUaUserTable : public QWidget
{
    Q_OBJECT

public:
    explicit QUaUserTable(QWidget *parent = nullptr);
    ~QUaUserTable();

	// TODO : pass in logged user (permissions, disable everything if no logged user)

	QUaAccessControl * accessControl() const;
	void setAccessControl(QUaAccessControl * ac);

	// table headers
	enum class Headers
	{
		Name      = 0,
		Role      = 1,
		Actions   = 2,
		Invalid   = 3
	};
	Q_ENUM(Headers)

signals:
	void userSelectionChanged(QUaUser * userPrev, QUaUser * userCurr);

private slots:
    void on_pushButtonAdd_clicked();

private:
    Ui::QUaUserTable *ui;
	QUaAccessControl    * m_ac;
	QStandardItemModel    m_modelUsers;
	QSortFilterProxyModel m_proxyUsers;

	void showNewUserDialog(QUaAcCommonDialog &dialog);
	QStandardItem *  handleUserAdded(QUaUser * user);

	// static values
	static int PointerRole;
};

#endif // QUAUSERTABLE_H
