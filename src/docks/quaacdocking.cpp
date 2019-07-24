#include "quaacdocking.h"

#include <QMessageBox>
#include <QInputDialog>

QString QUaAcDocking::m_strEmpty = QObject::tr("Empty");

QUaAcDocking::QUaAcDocking(QMainWindow *parent) : QObject(parent)
{
	m_dockManager = new QAdDockManager(parent);
	this->saveCurrentLayout(QUaAcDocking::m_strEmpty);
}

QAdDockWidgetArea * QUaAcDocking::addDockWidget(
	const QString     &strName,
	const QAdDockArea &dockArea,
	QWidget           *widget,
	QAdDockWidgetArea *widgetArea/* = nullptr*/
)
{
	Q_ASSERT(!this->hasDockWidget(strName));
	if (this->hasDockWidget(strName))
	{
		// TODO : remove old, put new?
		return nullptr;
	}
	auto pDock = new QAdDockWidget(strName, m_dockManager);
	pDock->setWidget(widget);
	return m_dockManager->addDockWidget(dockArea, pDock, widgetArea);
}

void QUaAcDocking::removeDockWidget(const QString & strName)
{
	auto widget = m_dockManager->findDockWidget(strName);
	if (!widget)
	{
		return;
	}
	m_dockManager->removeDockWidget(widget);
}

bool QUaAcDocking::hasDockWidget(const QString & strName)
{
	return m_dockManager->dockWidgetsMap().contains(strName);
}

QString QUaAcDocking::currentLayout() const
{
	return m_currLayout;
}

void QUaAcDocking::setEmptyLayout()
{
	this->setCurrentLayout(QUaAcDocking::m_strEmpty);
}

bool QUaAcDocking::layoutExists(const QString &strLayout) const
{
	return m_mapLayouts.contains(strLayout);
}

void QUaAcDocking::saveCurrentLayout(const QString & strLayout)
{
	if (this->layoutExists(strLayout))
	{
		m_mapLayouts[strLayout] = m_dockManager->saveState();
		emit this->layoutUpdated(strLayout);
	}
	else
	{
		m_mapLayouts.insert(strLayout, m_dockManager->saveState());
		emit this->layoutAdded(strLayout);
	}
}

void QUaAcDocking::removeLayout(const QString & strLayout)
{
	Q_ASSERT(this->layoutExists(strLayout));
	if (!this->layoutExists(strLayout))
	{
		return;
	}
	m_mapLayouts.remove(strLayout);
	emit this->layoutRemoved(strLayout);
}

void QUaAcDocking::setCurrentLayout(const QString & strLayout)
{
	Q_ASSERT(this->layoutExists(strLayout));
	if (!this->layoutExists(strLayout))
	{
		return;
	}
	m_currLayout = strLayout;
	m_dockManager->restoreState(m_mapLayouts.value(m_currLayout));
	emit this->currentLayoutChanged(m_currLayout);
}


void QUaAcDocking::on_saveLayout()
{
	// check is empty
	if (this->currentLayout().compare(QUaAcDocking::m_strEmpty, Qt::CaseInsensitive) == 0)
	{
		this->on_saveAsLayout();
		return;
	}
	this->saveCurrentLayout(this->currentLayout());
}

void QUaAcDocking::on_saveAsLayout()
{
	bool ok;
	QString strLayout = QInputDialog::getText(
		m_dockManager,
		tr("Save Layout As..."),
		tr("Layout Name :"),
		QLineEdit::Normal,
		"",
		&ok
	);
	if (!ok && strLayout.isEmpty())
	{
		return;
	}

	// TODO : validate text format ?

	// check is empty
	if (strLayout.compare(QUaAcDocking::m_strEmpty, Qt::CaseInsensitive) == 0)
	{
		QMessageBox::critical(
			m_dockManager,
			tr("Layout Name Error"),
			tr("Layout cannot be named %1.").arg(QUaAcDocking::m_strEmpty),
			QMessageBox::StandardButton::Ok
		);
		this->on_saveAsLayout();
		return;
	}
	// check exists
	if (this->layoutExists(strLayout))
	{
		auto res = QMessageBox::question(
			m_dockManager,
			tr("Save Layout"),
			tr("Layout %1 exists.\nWould you like to overwrite it?").arg(strLayout),
			QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No);
		if (res != QMessageBox::StandardButton::Yes)
		{
			return;
		}
	}
	this->saveCurrentLayout(strLayout);
	// set new layout
	this->setCurrentLayout(strLayout);
}

void QUaAcDocking::on_removeLayout()
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
	this->setCurrentLayout(QUaAcDocking::m_strEmpty);
}