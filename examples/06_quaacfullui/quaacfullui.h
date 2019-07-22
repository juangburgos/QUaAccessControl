#ifndef QUAACFULLUI_H
#define QUAACFULLUI_H

#include <QMainWindow>

namespace Ui {
class QUaAcFullUi;
}

#include <QUaServer>

class QUaUser;
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
	QUaServer   m_server;
	bool        m_deleting;
	QString     m_strSecret;
	QUaUser    *m_loggedUser;
	// widgets
	QAdDockManager           * m_dockManager;
	QUaUserTable             * m_userTable  ;
	QUaRoleTable             * m_roleTable  ;
	QUaPermissionsTable      * m_permsTable ;
	QUaUserWidgetEdit        * m_userWidget ;
	QUaRoleWidgetEdit        * m_roleWidget ;
	QUaPermissionsWidgetEdit * m_permsWidget;

	void setupInfoModel();
	void createWidgetsInDocks();
	void setupUserWidgets();
	void setupRoleWidgets();
	void setupPermsWidgets();
};

#endif // QUAACFULLUI_H
