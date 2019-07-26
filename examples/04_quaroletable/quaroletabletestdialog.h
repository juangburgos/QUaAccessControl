#ifndef QUAROLETABLETESTDIALOG_H
#define QUAROLETABLETESTDIALOG_H

#include <QDialog>

#include <QUaServer>

class QUaUser;
class QUaRole;
class QUaAcCommonDialog;

namespace Ui {
class QUaRoleTableTestDialog;
}

class QUaRoleTableTestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QUaRoleTableTestDialog(QWidget *parent = nullptr);
    ~QUaRoleTableTestDialog();

signals:
	void loggedUserChanged(QUaUser * user);

private slots:
    void on_pushButtonImport_clicked();

    void on_pushButtonExport_clicked();

    void on_pushButtonLogin_clicked();

    void on_pushButtonLogout_clicked();

	void on_loggedUserChanged(QUaUser * user);

private:
    Ui::QUaRoleTableTestDialog *ui;
	QUaServer   m_server;
	QString     m_strSecret;
	QUaUser    *m_loggedUser;
	bool        m_deleting;

	QList<QMetaObject::Connection> m_connections;

	QUaUser * loggedUser() const;
	void      setLoggedUser(QUaUser * user);

	void login();
	void logout();
	void showCreateRootUserDialog(QUaAcCommonDialog &dialog);
	void showUserCredentialsDialog(QUaAcCommonDialog &dialog);

	void clearApplication();

	void clearWidgetRoleEdit();
	void bindWidgetRoleEdit(QUaRole * role);
	void updateWidgetRoleEditPermissions(QUaUser * user);
};

#endif // QUAROLETABLETESTDIALOG_H
