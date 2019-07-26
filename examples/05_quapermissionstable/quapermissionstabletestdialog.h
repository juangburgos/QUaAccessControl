#ifndef QUAPERMISSIONSTABLETESTDIALOG_H
#define QUAPERMISSIONSTABLETESTDIALOG_H

#include <QDialog>

#include <QUaServer>

class QUaUser;
class QUaRole;
class QUaPermissions;
class QUaAcCommonDialog;

namespace Ui {
class QUaPermissionsTableTestDialog;
}

class QUaPermissionsTableTestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QUaPermissionsTableTestDialog(QWidget *parent = nullptr);
    ~QUaPermissionsTableTestDialog();

signals:
	void loggedUserChanged(QUaUser * user);

private slots:
    void on_pushButtonImport_clicked();

    void on_pushButtonExport_clicked();

    void on_pushButtonLogin_clicked();

    void on_pushButtonLogout_clicked();

	void on_loggedUserChanged(QUaUser * user);

private:
    Ui::QUaPermissionsTableTestDialog *ui;
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

	void clearWidgetPermissionsEdit();
	void bindWidgetPermissionsEdit(QUaPermissions * perms);
	void updateWidgetPermissionsEditPermissions(QUaUser * user);
};

#endif // QUAPERMISSIONSTABLETESTDIALOG_H
