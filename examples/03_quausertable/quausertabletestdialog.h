#ifndef QUAUSERTABLETESTDIALOG_H
#define QUAUSERTABLETESTDIALOG_H

#include <QDialog>

#include <QUaServer>

class QUaUser;
class QUaAcCommonDialog;

namespace Ui {
class QUaUserTableTestDialog;
}

class QUaUserTableTestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QUaUserTableTestDialog(QWidget *parent = nullptr);
    ~QUaUserTableTestDialog();

signals:
	void loggedUserChanged(QUaUser * user);

private slots:
    void on_pushButtonImport_clicked();

    void on_pushButtonExport_clicked();

    void on_pushButtonLogin_clicked();

    void on_pushButtonLogout_clicked();

	void on_loggedUserChanged(QUaUser * user);

private:
    Ui::QUaUserTableTestDialog *ui;
	QUaServer   m_server;
	QString     m_strSecret;
	QUaUser    *m_loggedUser;
	bool        m_deleting;

	QList<QMetaObject::Connection> m_connections;

	QUaUser * loggedUser() const;
	void      setLoggedUser(QUaUser * user);

	void login();
	void logout();
	void showCreateRootUserDialog (QUaAcCommonDialog &dialog);
	void showUserCredentialsDialog(QUaAcCommonDialog &dialog);

	void clearApplication();

	void clearWidgetUserEdit();
	void bindWidgetUserEdit(QUaUser * user);
	void updateWidgetUserEditPermissions(QUaUser * user);
};

#endif // QUAUSERTABLETESTDIALOG_H
