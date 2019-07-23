#ifndef QUAACFULLUI_H
#define QUAACFULLUI_H

#include <QMainWindow>
#include <QMenuBar>

namespace Ui {
class QUaAcFullUi;
}

#include <QUaServer>

class QUaUser;
class QUaRole;
class QUaPermissions;
class QUaAcCommonDialog;

#include <DockManager.h>
#include <DockWidget.h>
#include <DockAreaWidget.h>

namespace QAd = ads;
typedef QAd::CDockManager QAdDockManager;
typedef QAd::CDockWidget  QAdDockWidget;

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

private:
    Ui::QUaAcFullUi *ui;
	// opc ua
	QUaServer   m_server    ;
	bool        m_deleting  ;
	QString     m_strSecret ;
	QString     m_strTitle  ;
	QUaUser   * m_loggedUser;
	// widgets
	QAdDockManager           * m_dockManager;
	QUaUserTable             * m_userTable  ;
	QUaRoleTable             * m_roleTable  ;
	QUaPermissionsTable      * m_permsTable ;
	QUaUserWidgetEdit        * m_userWidget ;
	QUaRoleWidgetEdit        * m_roleWidget ;
	QUaPermissionsWidgetEdit * m_permsWidget;

	void setupInfoModel      ();
	void createWidgetsInDocks();
	void setupUserWidgets    ();
	void setupRoleWidgets    ();
	void setupPermsWidgets   ();
	void setupMenuBar        ();

	void openFile();
	void saveFile();
	void closeFile();

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
};

#endif // QUAACFULLUI_H
