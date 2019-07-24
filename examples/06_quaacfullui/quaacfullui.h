#ifndef QUAACFULLUI_H
#define QUAACFULLUI_H

#include <QMainWindow>
#include <QMenuBar>

#include <QUaServer>

#include <QUaAcDocking>

namespace Ui {
class QUaAcFullUi;
}

class QUaUser;
class QUaRole;
class QUaPermissions;
class QUaAcCommonDialog;

class QUaUserTable;
class QUaRoleTable;
class QUaPermissionsTable;
class QUaUserWidgetEdit;
class QUaRoleWidgetEdit;
class QUaPermissionsWidgetEdit;

class QUaAcFullUi : public QMainWindow
{
    Q_OBJECT

public:
    explicit QUaAcFullUi(QWidget *parent = nullptr);
    ~QUaAcFullUi();

signals:
	void loggedUserChanged(QUaUser * user);

private slots:
	void on_loggedUserChanged(QUaUser * user);

	void on_newConfig();
	void on_openConfig();
	void on_saveConfig();
	void on_closeConfig();

	void on_currentLayoutChanged(const QString &strLayout);

private:
    Ui::QUaAcFullUi *ui;

	// window members
	QUaServer        m_server     ;
	bool             m_deleting   ;
	QString          m_strSecret  ;
	QString          m_strTitle   ;
	QUaUser        * m_loggedUser ;
	QUaAcDocking   * m_dockManager;

	// ac widgets
	QUaUserTable             * m_userTable  ;
	QUaRoleTable             * m_roleTable  ;
	QUaPermissionsTable      * m_permsTable ;
	QUaUserWidgetEdit        * m_userWidget ;
	QUaRoleWidgetEdit        * m_roleWidget ;
	QUaPermissionsWidgetEdit * m_permsWidget;

	void setupInfoModel      ();
	void createAcWidgetsDocks();
	void setupUserWidgets    ();
	void setupRoleWidgets    ();
	void setupPermsWidgets   ();
	void setupMenuBar        ();
	void setupNativeDocks    ();

	QUaAccessControl * accessControl() const;

	QUaUser * loggedUser() const;
	void      setLoggedUser(QUaUser * user);

	void login ();
	void logout();
	void showCreateRootUserDialog (QUaAcCommonDialog &dialog);
	void showUserCredentialsDialog(QUaAcCommonDialog &dialog);

	QList<QMetaObject::Connection> m_connsUserWidget;
	void clearWidgetUserEdit();
	void bindWidgetUserEdit(QUaUser * user);
	void setWidgetUserEditPermissions(QUaUser * user);

	QList<QMetaObject::Connection> m_connsRoleWidget;
	void clearWidgetRoleEdit();
	void bindWidgetRoleEdit(QUaRole * role);
	void setWidgetRoleEditPermissions(QUaUser * user);

	QList<QMetaObject::Connection> m_connsPermsWidget;
	void clearWidgetPermissionsEdit();
	void bindWidgetPermissionsEdit(QUaPermissions * perms);
	void setWidgetPermissionsEditPermissions(QUaUser * user);


	static QString m_strAppName;
	static QString m_strUntitiled;
	static QString m_strDefault;

	// to find children
	static QString m_strHelpMenu;
	static QString m_strTopDock;
};

#endif // QUAACFULLUI_H
