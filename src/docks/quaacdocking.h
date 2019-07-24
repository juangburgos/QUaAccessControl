#ifndef QUAACDOCKING_H
#define QUAACDOCKING_H

#include <QMainWindow>

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

typedef QByteArray QUaAcLayoutsValue;
typedef QMap<QString, QUaAcLayoutsValue> QUaAcLayouts;
typedef QMapIterator<QString, QUaAcLayoutsValue> QUaAcLayoutsIter;

class QUaAcDocking : public QObject
{
    Q_OBJECT
public:
    explicit QUaAcDocking(QMainWindow *parent);

	// widget management

	QAdDockWidgetArea * addDockWidget(
		const QString     &strWidgetName,
		const QAdDockArea &dockArea,
		QWidget           *widget,
		QAdDockWidgetArea *widgetArea = nullptr
	);

	void removeDockWidget(const QString &strWidgetName);

	bool hasDockWidget(const QString &strWidgetName);

	QMenu * widgetsMenu();

	// layout management

	QString currentLayout () const;
	void    setEmptyLayout();
	bool    hasLayout     (const QString &strLayoutName) const;
	void    saveLayout    (const QString &strLayoutName);
	void    removeLayout  (const QString &strLayoutName);
	void    setLayout     (const QString &strLayoutName);

	QMenu * layoutsMenu();

signals:
	void widgetAdded         (const QString &strWidgetName);
	void widgetRemoved       (const QString &strWidgetName);

	void layoutAdded         (const QString &strLayoutName);
	void layoutUpdated       (const QString &strLayoutName);
	void layoutRemoved       (const QString &strLayoutName);
	void currentLayoutChanged(const QString &strLayoutName);

public slots:
	void saveCurrentLayout  ();
	void saveAsCurrentLayout();
	void removeCurrentLayout();

private slots:
	void on_widgetAdded  (const QString &strWidgetName);
	void on_widgetRemoved(const QString &strWidgetName);

	void on_layoutAdded  (const QString &strLayoutName);
	void on_layoutRemoved(const QString &strLayoutName);

private:
	QAdDockManager * m_dockManager;
	QUaAcLayouts     m_mapLayouts;
	QString          m_currLayout;

	QMenu          * m_widgetsMenu;
	QMenu          * m_layoutsMenu;

	void saveCurrentLayoutInternal(const QString &strLayoutName);

	static QString m_strEmpty;
};

#endif // QUAACDOCKING_H