#ifndef QUAACDOCKING_H
#define QUAACDOCKING_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include <DockManager.h>
#include <DockWidget.h>
#include <DockAreaWidget.h>

#include <QMap>
#include <QMapIterator>
#include <QByteArray>
#include <QString>
#include <QMenu>

namespace QAd = ads;
typedef QAd::CDockManager    QAdDockManager;
typedef QAd::CDockWidget     QAdDockWidget;
typedef QAd::DockWidgetArea  QAdDockArea;
typedef QAd::CDockAreaWidget QAdDockWidgetArea;

typedef std::function<void(void)> QAdWidgetEditFunc;

class QUaAccessControl;
class QUaUser;
class QUaPermissions;

struct QUaAcLayoutsValue
{
	QByteArray      byteState;
	QUaPermissions *permsObject;
};

//typedef QByteArray QUaAcLayoutsValue;
typedef QMap<QString, QUaAcLayoutsValue> QUaAcLayouts;
typedef QMapIterator<QString, QUaAcLayoutsValue> QUaAcLayoutsIter;

typedef QMap<QString, QUaPermissions*> QUaAcWidgetPerms;
typedef QMapIterator<QString, QUaPermissions*> QUaAcWidgetPermsIter;

class QUaAcDocking : public QObject
{
    Q_OBJECT
public:
    explicit QUaAcDocking(QMainWindow           * parent, 
		                  QStandardItemModel    * permsModel, 
		                  QSortFilterProxyModel * permsFilter);

	// widget management

	QAdDockWidgetArea * addDockWidget(
		const QString     &strWidgetName,
		const QAdDockArea &dockArea,
		QWidget           *widget,
		QWidget           *widgetEdit   = nullptr,
		QAdWidgetEditFunc  editCallback = nullptr,
		QAdDockWidgetArea *widgetArea   = nullptr
	);

	void removeDockWidget(const QString &strWidgetName);

	bool hasDockWidget(const QString &strWidgetName);

	QList<QString> widgetNames() const;

	QMenu * widgetsMenu();

	bool              hasWidgetPermissions(const QString &strWidgetName) const;
	QUaPermissions  * widgetPermissions   (const QString &strWidgetName) const;
	void              setWidgetPermissions(const QString &strWidgetName,
		                                   QUaPermissions * permissions);

	// TODO : permissions to set permissions, or display menus (widget list permissions)

	// layout management

	QString currentLayout () const;
	void    setEmptyLayout();
	bool    hasLayout     (const QString &strLayoutName) const;
	void    saveLayout    (const QString &strLayoutName);
	void    removeLayout  (const QString &strLayoutName);
	void    setLayout     (const QString &strLayoutName);

	QUaAcLayouts   layouts() const;
	QList<QString> layoutNames() const;

	QMenu * layoutsMenu();

	bool              hasLayoutPermissions(const QString &strLayoutName) const;
	QUaPermissions  * layoutPermissions   (const QString &strLayoutName) const;
	void              setLayoutPermissions(const QString &strLayoutName,
		                                   QUaPermissions * permissions);

	// TODO : permissions to set permissions, or display menus (layout list permissions)

signals:
	void widgetAdded             (const QString &strWidgetName);
	void widgetRemoved           (const QString &strWidgetName);
	void widgetPermissionsChanged(const QString &strWidgetName, QUaPermissions * permissions);

	void layoutAdded             (const QString &strLayoutName);
	void layoutUpdated           (const QString &strLayoutName);
	void layoutRemoved           (const QString &strLayoutName);
	void currentLayoutChanged    (const QString &strLayoutName);
	void layoutPermissionsChanged(const QString &strLayoutName, QUaPermissions * permissions);

public slots:
	void saveCurrentLayout  ();
	void saveAsCurrentLayout();
	void removeCurrentLayout();

	void on_loggedUserChanged(QUaUser * user);

private slots:
	void on_widgetAdded  (const QString &strWidgetName);
	void on_widgetRemoved(const QString &strWidgetName);

	void on_layoutAdded  (const QString &strLayoutName);
	void on_layoutRemoved(const QString &strLayoutName);

private:
	QAdDockManager * m_dockManager;
	QUaAcWidgetPerms m_mapWidgetPerms;
	QUaAcLayouts     m_mapLayouts;
	QString          m_currLayout;

	QMenu          * m_widgetsMenu;
	QMenu          * m_layoutsMenu;

	QUaUser        * m_loggedUser;

	QStandardItemModel    * m_modelPerms;
	QSortFilterProxyModel * m_proxyPerms;

	void saveCurrentLayoutInternal(const QString &strLayoutName);
	
	void updateLayoutPermissions();
	void updateWidgetPermissions();
	void updateLayoutPermissions(const QString &strLayoutName, QUaPermissions * permissions);
	void updateWidgetPermissions(const QString &strWidgetName, QUaPermissions * permissions);

	static QString m_strEmpty;
};

#endif // QUAACDOCKING_H