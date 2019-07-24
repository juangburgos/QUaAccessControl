#include "quaacdocking.h"

#include <QMessageBox>
#include <QInputDialog>

QString QUaAcDocking::m_strEmpty = QObject::tr("Empty");

QUaAcDocking::QUaAcDocking(QMainWindow *parent) : QObject(parent)
{
	m_dockManager = new QAdDockManager(parent);
	m_widgetsMenu = new QMenu(tr("Widgets"), m_dockManager);
	m_layoutsMenu = new QMenu(tr("Layouts"), m_dockManager);
	// signals and slots
	QObject::connect(this, &QUaAcDocking::widgetAdded  , this, &QUaAcDocking::on_widgetAdded  );
	QObject::connect(this, &QUaAcDocking::widgetRemoved, this, &QUaAcDocking::on_widgetRemoved);
	QObject::connect(this, &QUaAcDocking::layoutAdded  , this, &QUaAcDocking::on_layoutAdded  );
	QObject::connect(this, &QUaAcDocking::layoutRemoved, this, &QUaAcDocking::on_layoutRemoved);
	this->saveCurrentLayoutInternal(QUaAcDocking::m_strEmpty);
}

QAdDockWidgetArea * QUaAcDocking::addDockWidget(
	const QString     &strWidgetName,
	const QAdDockArea &dockArea,
	QWidget           *widget,
	QAdDockWidgetArea *widgetArea/* = nullptr*/
)
{
	Q_ASSERT(!this->hasDockWidget(strWidgetName));
	if (this->hasDockWidget(strWidgetName))
	{
		// TODO : remove old, put new?
		return nullptr;
	}
	auto pDock = new QAdDockWidget(strWidgetName, m_dockManager);
	pDock->setWidget(widget);
	auto wAreaNew = m_dockManager->addDockWidget(dockArea, pDock, widgetArea); 
	emit this->widgetAdded(strWidgetName);
	return wAreaNew;
}

void QUaAcDocking::removeDockWidget(const QString & strWidgetName)
{
	auto widget = m_dockManager->findDockWidget(strWidgetName);
	if (!widget)
	{
		return;
	}
	emit this->widgetRemoved(strWidgetName); // NOTE : before, so still can find it in callback
	m_dockManager->removeDockWidget(widget);
}

bool QUaAcDocking::hasDockWidget(const QString & strWidgetName)
{
	return m_dockManager->dockWidgetsMap().contains(strWidgetName);
}

QMenu * QUaAcDocking::widgetsMenu()
{
	return m_widgetsMenu;
}

QString QUaAcDocking::currentLayout() const
{
	return m_currLayout;
}

void QUaAcDocking::setEmptyLayout()
{
	this->setLayout(QUaAcDocking::m_strEmpty);
}

bool QUaAcDocking::hasLayout(const QString &strLayoutName) const
{
	return m_mapLayouts.contains(strLayoutName);
}

void QUaAcDocking::saveLayout(const QString & strLayoutName)
{
	// check is empty
	Q_ASSERT((strLayoutName.compare(QUaAcDocking::m_strEmpty, Qt::CaseInsensitive) != 0));
	if (strLayoutName.compare(QUaAcDocking::m_strEmpty, Qt::CaseInsensitive) == 0)
	{
		return;
	}
	this->saveCurrentLayoutInternal(strLayoutName);
}

void QUaAcDocking::saveCurrentLayoutInternal(const QString & strLayoutName)
{
	if (this->hasLayout(strLayoutName))
	{
		m_mapLayouts[strLayoutName] = m_dockManager->saveState();
		emit this->layoutUpdated(strLayoutName);
	}
	else
	{
		m_mapLayouts.insert(strLayoutName, m_dockManager->saveState());
		emit this->layoutAdded(strLayoutName);
	}
}

void QUaAcDocking::removeLayout(const QString & strLayoutName)
{
	Q_ASSERT(this->hasLayout(strLayoutName));
	if (!this->hasLayout(strLayoutName))
	{
		return;
	}
	m_mapLayouts.remove(strLayoutName);
	emit this->layoutRemoved(strLayoutName);
}

void QUaAcDocking::setLayout(const QString & strLayoutName)
{
	Q_ASSERT(this->hasLayout(strLayoutName));
	if (!this->hasLayout(strLayoutName))
	{
		return;
	}
	m_currLayout = strLayoutName;
	m_dockManager->restoreState(m_mapLayouts.value(m_currLayout));
	emit this->currentLayoutChanged(m_currLayout);
}

QMenu * QUaAcDocking::layoutsMenu()
{
	return m_layoutsMenu;
}


void QUaAcDocking::saveCurrentLayout()
{
	// check is empty
	if (this->currentLayout().compare(QUaAcDocking::m_strEmpty, Qt::CaseInsensitive) == 0)
	{
		this->saveAsCurrentLayout();
		return;
	}
	this->saveCurrentLayoutInternal(this->currentLayout());
}

void QUaAcDocking::saveAsCurrentLayout()
{
	bool ok;
	QString strLayoutName = QInputDialog::getText(
		m_dockManager,
		tr("Save Layout As..."),
		tr("Layout Name :"),
		QLineEdit::Normal,
		"",
		&ok
	);
	if (!ok && strLayoutName.isEmpty())
	{
		return;
	}

	// TODO : validate text format ?

	// check is empty
	if (strLayoutName.compare(QUaAcDocking::m_strEmpty, Qt::CaseInsensitive) == 0)
	{
		QMessageBox::critical(
			m_dockManager,
			tr("Layout Name Error"),
			tr("Layout cannot be named %1.").arg(QUaAcDocking::m_strEmpty),
			QMessageBox::StandardButton::Ok
		);
		this->saveAsCurrentLayout();
		return;
	}
	// check exists
	if (this->hasLayout(strLayoutName))
	{
		auto res = QMessageBox::question(
			m_dockManager,
			tr("Save Layout"),
			tr("Layout %1 exists.\nWould you like to overwrite it?").arg(strLayoutName),
			QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No);
		if (res != QMessageBox::StandardButton::Yes)
		{
			return;
		}
	}
	this->saveCurrentLayoutInternal(strLayoutName);
	// set new layout
	this->setLayout(strLayoutName);
}

void QUaAcDocking::removeCurrentLayout()
{
	// check is empty
	if (this->currentLayout().compare(QUaAcDocking::m_strEmpty, Qt::CaseInsensitive) == 0)
	{
		QMessageBox::critical(
			m_dockManager,
			tr("Remove Layout Error"),
			tr("Cannot remove %1 layout.").arg(QUaAcDocking::m_strEmpty),
			QMessageBox::StandardButton::Ok
		);
		return;
	}
	// ask for confirmation
	auto res = QMessageBox::warning(
		m_dockManager,
		tr("Delete Layout Confirmation"),
		tr("Are you sure you want to delete layout %1?").arg(this->currentLayout()),
		QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No
	);
	if (res != QMessageBox::StandardButton::Yes)
	{
		return;
	}
	// remove
	this->removeLayout(this->currentLayout());
	// initially empty
	this->setLayout(QUaAcDocking::m_strEmpty);
}

void QUaAcDocking::on_widgetAdded(const QString & strWidgetName)
{
	auto widget = m_dockManager->findDockWidget(strWidgetName);
	Q_ASSERT(widget);
	m_widgetsMenu->addAction(widget->toggleViewAction());
}

void QUaAcDocking::on_widgetRemoved(const QString & strWidgetName)
{
	auto widget = m_dockManager->findDockWidget(strWidgetName);
	Q_ASSERT(widget);
	m_widgetsMenu->removeAction(widget->toggleViewAction());
}

void QUaAcDocking::on_layoutAdded(const QString & strLayoutName)
{
	m_layoutsMenu->addAction(strLayoutName, this,
	[this, strLayoutName]() {
		this->setLayout(strLayoutName);
	})->setObjectName(strLayoutName);
}

void QUaAcDocking::on_layoutRemoved(const QString & strLayoutName)
{
	QAction * layoutAction = m_layoutsMenu->findChild<QAction*>(strLayoutName);
	Q_CHECK_PTR(layoutAction);
	m_layoutsMenu->removeAction(layoutAction);
}