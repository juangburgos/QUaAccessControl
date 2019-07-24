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
		const QString     &strName,
		const QAdDockArea &dockArea,
		QWidget           *widget,
		QAdDockWidgetArea *widgetArea = nullptr
	);

	void removeDockWidget(const QString &strName);

	bool hasDockWidget(const QString &strName);

	// layout management

	QString currentLayout() const;
	void setEmptyLayout();
	bool layoutExists     (const QString &strLayout) const;
	void saveCurrentLayout(const QString &strLayout);
	void removeLayout     (const QString &strLayout);
	void setCurrentLayout (const QString &strLayout);

signals:
	void layoutAdded         (const QString &strLayout);
	void layoutUpdated       (const QString &strLayout);
	void layoutRemoved       (const QString &strLayout);
	void currentLayoutChanged(const QString &strLayout);

public slots:
	void on_saveLayout  ();
	void on_saveAsLayout();
	void on_removeLayout();

private:
	QAdDockManager * m_dockManager;
	QUaAcLayouts     m_mapLayouts;
	QString          m_currLayout;

	static QString m_strEmpty;
};

#endif // QUAACDOCKING_H