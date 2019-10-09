#ifndef QUAACDOCKING_H
#define QUAACDOCKING_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QUaAcCommonWidgets>

#include <DockManager.h>
#include <DockWidget.h>
#include <DockAreaWidget.h>
#include <DockWidgetTab.h>

#include <QMap>
#include <QMapIterator>
#include <QByteArray>
#include <QString>
#include <QMenu>
#include <QDomDocument>
#include <QDomElement>

namespace QAd = ads;
typedef QAd::CDockManager    QAdDockManager;
typedef QAd::CDockWidget     QAdDockWidget;
typedef QAd::DockWidgetArea  QAdDockArea;
typedef QAd::CDockAreaWidget QAdDockWidgetArea;
typedef QAd::CDockWidgetTab  QAdDockWidgetTab;

typedef std::function<void(void)> QAdWidgetEditFunc;

class QUaAccessControl;
class QUaUser;
class QUaPermissions;

class QUaAcCommonDialog;

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
		                  QSortFilterProxyModel * permsFilter);

	// widget management

	QAdDockWidgetArea * addDock(
		const QString     &strDockPathName,
		const QAdDockArea &dockArea,
		QWidget           *widget,
		QWidget           *widgetEdit   = nullptr,
		QAdWidgetEditFunc  editCallback = nullptr,
		QAdDockWidgetArea *widgetArea   = nullptr
	);

	void removeDock(const QString &strDockName);

	bool hasDock(const QString &strDockName);

	QAdDockWidget * dock(const QString &strDockName) const;

	QList<QString> dockNames() const;

	bool isDockVisible(const QString &strDockName);
	bool setIsDockVisible(const QString &strDockName, const bool &visible);

	QMenu * docksMenu();

	bool              hasDockPermissions(const QString &strDockName) const;
	QUaPermissions  * dockPermissions   (const QString &strDockName) const;
	void              setDockPermissions(const QString &strDockName,
		                                 QUaPermissions * permissions);

	// over all list permissions
	// can read controls if user can list, create, save, remove or set permissions to individual widgets (hides top-elvel menu)
	// can write controls if user can set widget list permissions (this permissions)
	void setDockListPermissions(QUaPermissions * permissions);
	QUaPermissions * dockListPermissions() const;

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
	QUaAcLambdaFilterProxy * layoutsModel();

	bool              hasLayoutPermissions(const QString &strLayoutName) const;
	QUaPermissions  * layoutPermissions   (const QString &strLayoutName) const;
	void              setLayoutPermissions(const QString &strLayoutName,
		                                   QUaPermissions * permissions);

	// over all list permissions
	// can read controls if user can list, create, save, remove or set permissions to individual layouts (hides top-elvel menu)
	// can write controls if user can set layout list permissions (this permissions)
	void setLayoutListPermissions(QUaPermissions * permissions);
	QUaPermissions * layoutListPermissions() const;

	// XML import / export
	// NOTE : only layouts, widgets are serialized wherever they are handled (factory)

	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElement(QUaAccessControl * ac, QDomElement  & domElem, QString &strError);

	const static QString m_strXmlName;
	const static QString m_strXmlDockName;
	const static QString m_strXmlLayoutName;

signals:
	void aboutToChangeLayout();
	void currentLayoutChanged    (const QString &strLayoutName);
	void layoutPermissionsChanged(const QString &strLayoutName, QUaPermissions * permissions);
	void layoutListPermissionsChanged(QUaPermissions * permissions);

public slots:
	void saveCurrentLayout  ();
	void saveAsCurrentLayout();
	void removeCurrentLayout();

	void on_loggedUserChanged(QUaUser * user);

private:
	QAdDockManager * m_dockManager;
	QUaAcWidgetPerms m_mapDockPerms;
	QUaAcLayouts     m_mapLayouts;
	QString          m_currLayout;

	QMenu          * m_docksMenu;
	QMenu          * m_layoutsMenu;

	QUaUser        * m_loggedUser;

	QHash<QString, QUaAcCommonDialog*> m_mapDialogs;

	QUaPermissions * m_dockListPerms;
	QUaPermissions * m_layoutListPerms;

	QSortFilterProxyModel * m_proxyPerms;

	// layouts model for combobox
	QStandardItemModel     m_modelLayouts;
	QUaAcLambdaFilterProxy m_proxyLayouts;
	void setupLayoutsModel();

	void saveCurrentLayoutInternal(const QString &strLayoutName);

	void handleDockAdded  (const QStringList &strDockPathName, QMenu * menuParent, const int &index = 0);
	void handleDockRemoved(const QString &strDockName);

	void handleLayoutAdded  (const QString &strLayoutName);
	void handleLayoutRemoved(const QString &strLayoutName);
	void handleLayoutUpdated(const QString &strLayoutName);
	
	void updateLayoutPermissions();
	void updateDockPermissions();
	void updateLayoutPermissions(const QString &strLayoutName, QUaPermissions * permissions);
	void updateDockPermissions  (const QString &strDockName, QUaPermissions * permissions);

	void updateLayoutListPermissions();
	void updateDockListPermissions();

	QUaPermissions * findPermissions(QUaAccessControl * ac, const QString &strNodeId, QString &strError);

	const static QString m_strEmpty;
};

#endif // QUAACDOCKING_H