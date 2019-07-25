#include "quaacdocking.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QLayout>

#include <QUaAccessControl>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>

#include <QUaAcCommonDialog>
#include <QAdDockWidgetWrapper>
#include <QAdDockWidgetConfig>
#include <QUaDockWidgetPerms>

QString QUaAcDocking::m_strEmpty = QObject::tr("Empty");

QUaAcDocking::QUaAcDocking(QMainWindow *parent, QUaAccessControl *ac)
	: QObject(parent), m_ac(ac)
{
	Q_CHECK_PTR(parent);
	Q_CHECK_PTR(ac);
	m_loggedUser  = nullptr;
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
	QWidget           *widgetEdit  /*= nullptr*/,
	QAdWidgetEditFunc  editCallback/*= nullptr*/,
	QAdDockWidgetArea *widgetArea  /*= nullptr*/
)
{
	Q_ASSERT(!this->hasDockWidget(strWidgetName));
	if (this->hasDockWidget(strWidgetName))
	{
		// TODO : remove old, put new?
		return nullptr;
	}
	// create dock
	auto pDock = new QAdDockWidget(strWidgetName, m_dockManager);
	// create wrapper
	QAdDockWidgetWrapper * wrapper = new QAdDockWidgetWrapper(pDock);
	wrapper->setEditBarVisible(false);
	// set widget in wrapper
	wrapper->setWidget(widget);
	// set wrapper in dock
	pDock->setWidget(wrapper);
	// fix margins
	wrapper->layout()->setContentsMargins(3, 3, 3, 3);
	// add dock
	auto wAreaNew = m_dockManager->addDockWidget(dockArea, pDock, widgetArea); 
	emit this->widgetAdded(strWidgetName);
	// handle config button clicked
	QObject::connect(wrapper, &QAdDockWidgetWrapper::configClicked, pDock,
	[this, strWidgetName, widgetEdit, editCallback]() {
		// create config widget
		auto configTabWidget = new QAdDockWidgetConfig;
		if (widgetEdit)
		{
			// set and takes ownership
			configTabWidget->setConfigWidget(widgetEdit);
			// remove ownership from tab widget (else will be delated when dialog closed)
			widgetEdit->setParent(m_dockManager);
		}
		// create permissions widget
		auto permsTabWidget = new QUaDockWidgetPerms;

		// TODO : configure

		// set and takes ownership
		configTabWidget->setPermissionsWidget(permsTabWidget);
		// dialog
		QUaAcCommonDialog dialog(m_dockManager);
		dialog.setWindowTitle(tr("Config Widget"));
		dialog.setWidget(configTabWidget);
		// exec dialog
		int res = dialog.exec();
		if (res != QDialog::Accepted)
		{
			return;
		}

		// TODO : read permissions and set them for widget

		this->setWidgetPermissions(strWidgetName, nullptr);
		// call apply callback
		if (editCallback)
		{
			editCallback();
		}
	});
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

QList<QString> QUaAcDocking::widgetNames() const
{
	return m_dockManager->dockWidgetsMap().keys();
}

QMenu * QUaAcDocking::widgetsMenu()
{
	return m_widgetsMenu;
}

bool QUaAcDocking::hasWidgetPermissions(const QString & strWidgetName) const
{
	return m_mapWidgetPerms.contains(strWidgetName);
}

QUaPermissions * QUaAcDocking::widgetPermissions(const QString & strWidgetName) const
{
	return m_mapWidgetPerms.value(strWidgetName, nullptr);
}

void QUaAcDocking::setWidgetPermissions(const QString & strWidgetName, QUaPermissions * permissions)
{
	if (!permissions)
	{
		m_mapWidgetPerms.remove(strWidgetName);
		return;
	}
	// set permissions
	m_mapWidgetPerms[strWidgetName] = permissions;
	// update permissions
	this->updateWidgetPermissions(strWidgetName, permissions);
	// emit
	emit this->widgetPermissionsChanged(strWidgetName, permissions);
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
		m_mapLayouts[strLayoutName].byteState = m_dockManager->saveState();
		emit this->layoutUpdated(strLayoutName);
	}
	else
	{
		m_mapLayouts.insert(strLayoutName, { m_dockManager->saveState(), nullptr });
		emit this->layoutAdded(strLayoutName);
	}
}

void QUaAcDocking::updatePermissions()
{
	// update widgets permissions
	for (auto widgetName : this->widgetNames())
	{
		this->updateWidgetPermissions(widgetName, m_mapWidgetPerms.value(widgetName, nullptr));
	}

	// TODO : layouts
}

void QUaAcDocking::updateWidgetPermissions(const QString & strWidgetName, QUaPermissions * permissions)
{
	// update widget permissions iff current user valid
	auto widget = m_dockManager->findDockWidget(strWidgetName);
	Q_ASSERT(widget);
	Q_ASSERT(widget->toggleViewAction());
	auto wrapper = dynamic_cast<QAdDockWidgetWrapper*>(widget->widget());
	Q_ASSERT(wrapper);
	// read
	widget->toggleViewAction()->setVisible(permissions ? permissions->canUserRead(m_loggedUser) : m_loggedUser ? true : false);
	widget->toggleView(permissions ? permissions->canUserRead(m_loggedUser) && !widget->isClosed() : m_loggedUser ? !widget->isClosed() : false);
	// write (set permissions)
	wrapper->setEditBarVisible(permissions ? permissions->canUserWrite(m_loggedUser) : m_loggedUser ? true : false);
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
	// uncheck old layout action
	QAction * layActOld = m_layoutsMenu->findChild<QAction*>(this->currentLayout());
	Q_CHECK_PTR(layActOld);
	layActOld->setChecked(false);
	// set new layout
	m_currLayout = strLayoutName;
	m_dockManager->restoreState(m_mapLayouts.value(m_currLayout).byteState);
	// update permissions
	this->updatePermissions();
	// check new layout action
	QAction * layActNew = m_layoutsMenu->findChild<QAction*>(strLayoutName);
	Q_CHECK_PTR(layActNew);
	layActNew->setChecked(true);
	// emit
	emit this->currentLayoutChanged(m_currLayout);
}

QList<QString> QUaAcDocking::layoutNames() const
{
	return m_mapLayouts.keys();
}

QMenu * QUaAcDocking::layoutsMenu()
{
	return m_layoutsMenu;
}

bool QUaAcDocking::hasLayoutPermissions(const QString & strLayoutName) const
{
	if (!m_mapLayouts.contains(strLayoutName))
	{
		return false;
	}
	return m_mapLayouts[strLayoutName].permsObject;
}

QUaPermissions * QUaAcDocking::layoutPermissions(const QString & strLayoutName) const
{
	if (!this->hasLayoutPermissions(strLayoutName))
	{
		return nullptr;
	}
	return m_mapLayouts[strLayoutName].permsObject;
}

void QUaAcDocking::setLayoutPermissions(const QString & strLayoutName, QUaPermissions * permissions)
{
	Q_ASSERT(m_mapLayouts.contains(strLayoutName));
	if (!m_mapLayouts.contains(strLayoutName))
	{
		return;
	}
	m_mapLayouts[strLayoutName].permsObject = permissions;
	emit this->layoutPermissionsChanged(strLayoutName, permissions);
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

void QUaAcDocking::on_loggedUserChanged(QUaUser * user)
{
	m_loggedUser = user;
	// refresh (also updates permissions)
	this->setLayout(this->currentLayout());
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
	auto act = m_layoutsMenu->addAction(strLayoutName, this,
	[this, strLayoutName]() {
		this->setLayout(strLayoutName);
	});
	act->setObjectName(strLayoutName);
	act->setCheckable(true);
}

void QUaAcDocking::on_layoutRemoved(const QString & strLayoutName)
{
	QAction * layoutAction = m_layoutsMenu->findChild<QAction*>(strLayoutName);
	Q_CHECK_PTR(layoutAction);
	m_layoutsMenu->removeAction(layoutAction);
}
