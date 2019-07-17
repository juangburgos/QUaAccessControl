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

public:
    explicit QUaUserTable(QWidget *parent = nullptr);
    ~QUaUserTable();

	bool isAddVisible() const;
	void setAddVisible(const bool &isVisible);

	QUaAccessControl * accessControl() const;
	void setAccessControl(QUaAccessControl * ac);

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

	void showNewUserDialog(QUaAcCommonDialog &dialog);
	QStandardItem *  handleUserAdded(QUaUser * user);

	// static values
	static int PointerRole;
};

#endif // QUAUSERTABLE_H
