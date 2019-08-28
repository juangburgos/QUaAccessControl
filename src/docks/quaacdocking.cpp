#include "quaacdocking.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QLayout>

#include <QUaServer>
#include <QUaAccessControl>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>

#include <QUaAcCommonDialog>
#include <QAdDockWidgetWrapper>
#include <QAdDockWidgetConfig>
#include <QUaDockWidgetPerms>

const QString QUaAcDocking::m_strEmpty = QObject::tr("Empty");

const QString QUaAcDocking::m_strXmlName       = "QDockManager";
const QString QUaAcDocking::m_strXmlWidgetName = "QDockWidget";
const QString QUaAcDocking::m_strXmlLayoutName = "QDockLayout";

QUaAcDocking::QUaAcDocking(QMainWindow           * parent, 
		                   QSortFilterProxyModel * permsFilter)
	: QObject(parent), m_proxyPerms(permsFilter)
{
	Q_CHECK_PTR(parent);
	Q_CHECK_PTR(permsFilter);
	m_loggedUser      = nullptr;
	m_widgetListPerms = nullptr;
	m_layoutListPerms = nullptr;
	m_dockManager     = new QAdDockManager(parent);
	// widgets menu
	m_widgetsMenu   = new QMenu(tr("Widgets"), m_dockManager);
	m_widgetsMenu->addAction(tr("Permissions..."), 
	[this]() {
		// create permissions widget
		auto permsWidget = new QUaDockWidgetPerms;
		// configure perms widget combo
		permsWidget->setComboModel(m_proxyPerms);
		permsWidget->setPermissions(this->widgetListPermissions());
		// dialog
		QUaAcCommonDialog dialog(m_dockManager);
		dialog.setWindowTitle(tr("Widget List Permissions"));
		dialog.setWidget(permsWidget);
		// exec dialog
		int res = dialog.exec();
		if (res != QDialog::Accepted)
		{
			return;
		}
		// read permissions and set them for widget list
		this->setWidgetListPermissions(permsWidget->permissions());
	})->setObjectName("Permissions");
	m_widgetsMenu->addSeparator();
	m_layoutsMenu  = new QMenu(tr("Layouts"), m_dockManager);
	m_layoutsMenu->addAction(tr("Permissions..."), 
	[this]() {
		// create permissions widget
		auto permsWidget = new QUaDockWidgetPerms;
		// configure perms widget combo
		permsWidget->setComboModel(m_proxyPerms);
		permsWidget->setPermissions(this->layoutListPermissions());
		// dialog
		QUaAcCommonDialog dialog(m_dockManager);
		dialog.setWindowTitle(tr("Layout List Permissions"));
		dialog.setWidget(permsWidget);
		// exec dialog
		int res = dialog.exec();
		if (res != QDialog::Accepted)
		{
			return;
		}
		// read permissions and set them for layout list
		this->setLayoutListPermissions(permsWidget->permissions());
	})->setObjectName("Permissions");
	m_layoutsMenu->addSeparator();
	QMenu *menuLayouts = m_layoutsMenu->addMenu(tr("Show"));
	menuLayouts->setObjectName("Show");
	m_layoutsMenu->addSeparator();
	m_layoutsMenu->addAction(tr("Save")      , this, &QUaAcDocking::saveCurrentLayout  )->setObjectName("Save");
	m_layoutsMenu->addAction(tr("Save As..."), this, &QUaAcDocking::saveAsCurrentLayout)->setObjectName("SaveAs");
	m_layoutsMenu->addSeparator()->setObjectName("Separator");
	m_layoutsMenu->addAction(tr("Remove")    , this, &QUaAcDocking::removeCurrentLayout)->setObjectName("Remove");
	// signals and slots
	QObject::connect(this, &QUaAcDocking::layoutAdded  , this, &QUaAcDocking::on_layoutAdded  );
	QObject::connect(this, &QUaAcDocking::layoutRemoved, this, &QUaAcDocking::on_layoutRemoved);
	this->saveCurrentLayoutInternal(QUaAcDocking::m_strEmpty);
}

QAdDockWidgetArea * QUaAcDocking::addDockWidget(
	const QString     &strWidgetPathName,
	const QAdDockArea &dockArea,
	QWidget           *widget,
	QWidget           *widgetEdit  /*= nullptr*/,
	QAdWidgetEditFunc  editCallback/*= nullptr*/,
	QAdDockWidgetArea *widgetArea  /*= nullptr*/
)
{
	// get only name from path, name must be unique
	QStringList listPathParts = strWidgetPathName.split("/");
	QString     strWidgetName = listPathParts.last();
	// check does not exists
	Q_ASSERT_X(!this->hasDockWidget(strWidgetName), "addDockWidget", "Repeated Widget Name not supported.");
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
	// fix size (in advance)
	auto geo = pDock->geometry();
	geo.setWidth (1.1 * wrapper->width());
	geo.setHeight(1.1 * wrapper->height());
	pDock->setGeometry(geo);
	// set wrapper in dock
	pDock->setWidget(wrapper);
	// fix margins
	wrapper->layout()->setContentsMargins(3, 3, 3, 3);
	// add dock
	auto wAreaNew = m_dockManager->addDockWidget(dockArea, pDock, widgetArea); 
	// add to menu and tree model
	this->handleWidgetAdded(listPathParts, m_widgetsMenu);
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
		// configure perms widget combo
		permsTabWidget->setComboModel(m_proxyPerms);
		permsTabWidget->setPermissions(this->widgetPermissions(strWidgetName));
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
		// read permissions and set them for widget
		this->setWidgetPermissions(strWidgetName, permsTabWidget->permissions());
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
	this->handleWidgetRemoved(strWidgetName);
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
	}
	else
	{
		// set permissions
		m_mapWidgetPerms[strWidgetName] = permissions;
	}
	// update permissions
	this->updateWidgetPermissions(strWidgetName, permissions);
}

void QUaAcDocking::setWidgetListPermissions(QUaPermissions * permissions)
{
	m_widgetListPerms = permissions;
	this->updateWidgetListPermissions();
}

QUaPermissions * QUaAcDocking::widgetListPermissions() const
{
	return m_widgetListPerms;
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
		// when created, set current user's permissions if any, by default
		if (m_loggedUser && m_loggedUser->permissionsObject())
		{
			this->setLayoutPermissions(strLayoutName, m_loggedUser->permissionsObject());
		}
	}
}

void QUaAcDocking::updateLayoutPermissions()
{
	QUaAcLayoutsIter i(m_mapLayouts);
	while (i.hasNext()) {
		i.next();
		this->updateLayoutPermissions(i.key(), i.value().permsObject);
	}
}

void QUaAcDocking::updateWidgetPermissions()
{
	// update widgets permissions
	for (auto widgetName : this->widgetNames())
	{
		this->updateWidgetPermissions(widgetName, m_mapWidgetPerms.value(widgetName, nullptr));
	}
}

void QUaAcDocking::updateLayoutPermissions(const QString & strLayoutName, QUaPermissions * permissions)
{
	QMenu *menuLayouts = m_layoutsMenu->findChild<QMenu*>("Show");
	Q_CHECK_PTR(menuLayouts);
	QAction * layActOld = menuLayouts->findChild<QAction*>(strLayoutName);
	Q_CHECK_PTR(layActOld);
	layActOld->setVisible(!m_loggedUser ? false : !permissions ? true : permissions->canUserRead(m_loggedUser));
	// check if current
	if (strLayoutName.compare(m_currLayout, Qt::CaseInsensitive) != 0)
	{
		return;
	}
	// show/hide layout menu actions
	bool canWrite     = !m_loggedUser ? false : !permissions ? true : permissions->canUserWrite(m_loggedUser);
	m_layoutsMenu->findChild<QAction*>("Save"     )->setVisible(canWrite);
	// NOTE : still allow read_only user to create own (save as). If this is not desired then remove write permissions to layout list permissions
	m_layoutsMenu->findChild<QAction*>("Separator")->setVisible(canWrite);
	m_layoutsMenu->findChild<QAction*>("Remove"   )->setVisible(canWrite);
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
	bool canRead = !m_loggedUser ? false : !permissions ? true : permissions->canUserRead(m_loggedUser);
	widget->toggleViewAction()->setVisible(canRead);
	bool isOpen  = !widget->isClosed();
	bool setOpen = !m_loggedUser ? false : !permissions ? isOpen : permissions->canUserRead(m_loggedUser) && isOpen;
	widget->toggleView(setOpen);
	// write (set permissions)
	bool canWrite = !m_loggedUser ? false : !permissions ? true : permissions->canUserWrite(m_loggedUser);
	wrapper->setEditBarVisible(canWrite);
}

void QUaAcDocking::updateLayoutListPermissions()
{
	// show/hide root menu
	bool canRead = !m_loggedUser ? false : !m_layoutListPerms ? true : m_layoutListPerms->canUserRead(m_loggedUser);
	m_layoutsMenu->menuAction()->setVisible(canRead);
	// show/hide layout list permissions action
	bool canWrite = !m_loggedUser ? false : !m_layoutListPerms ? true : m_layoutListPerms->canUserWrite(m_loggedUser);
	m_layoutsMenu->findChild<QAction*>("Permissions")->setVisible(canWrite);
	// NOTE : write permissions affect wherever new layouts are created (e.g. QAdDockLayoutBar)
}

void QUaAcDocking::updateWidgetListPermissions()
{
	// show/hide root menu
	bool canRead = !m_loggedUser ? false : !m_widgetListPerms ? true : m_widgetListPerms->canUserRead(m_loggedUser);
	m_widgetsMenu->menuAction()->setVisible(canRead);
	// show/hide widget list permissions action
	bool canWrite = !m_loggedUser ? false : !m_widgetListPerms ? true : m_widgetListPerms->canUserWrite(m_loggedUser);
	m_widgetsMenu->findChild<QAction*>("Permissions")->setVisible(canWrite);
	// NOTE : write permissions affect wherever new widgets are created
}

void QUaAcDocking::removeLayout(const QString & strLayoutName)
{
	Q_ASSERT(this->hasLayout(strLayoutName));
	if (!this->hasLayout(strLayoutName))
	{
		return;
	}
	// check is empty
	if (strLayoutName.compare(QUaAcDocking::m_strEmpty, Qt::CaseInsensitive) == 0)
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
	QMenu *menuLayouts = m_layoutsMenu->findChild<QMenu*>("Show");
	Q_CHECK_PTR(menuLayouts);
	// uncheck old layout action
	QAction * layActOld = menuLayouts->findChild<QAction*>(this->currentLayout());
	Q_CHECK_PTR(layActOld);
	layActOld->setChecked(false);
	// set new layout
	m_currLayout = strLayoutName;
	m_dockManager->restoreState(m_mapLayouts.value(m_currLayout).byteState);
	// update permissions
	this->updateWidgetPermissions();
	// check new layout action
	QAction * layActNew = menuLayouts->findChild<QAction*>(strLayoutName);
	Q_CHECK_PTR(layActNew);
	layActNew->setChecked(true);
	// update menu permissions
	this->updateLayoutPermissions(strLayoutName, m_mapLayouts[strLayoutName].permsObject);
	// emit
	emit this->currentLayoutChanged(m_currLayout);
}

QUaAcLayouts QUaAcDocking::layouts() const
{
	return m_mapLayouts;
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
	this->updateLayoutPermissions(strLayoutName, permissions);
}

void QUaAcDocking::setLayoutListPermissions(QUaPermissions * permissions)
{
	m_layoutListPerms = permissions;
	this->updateLayoutListPermissions();
	emit this->layoutListPermissionsChanged(permissions);
}

QUaPermissions * QUaAcDocking::layoutListPermissions() const
{
	return m_layoutListPerms;
}

QDomElement QUaAcDocking::toDomElement(QDomDocument & domDoc) const
{
	// add element
	QDomElement elemDock = domDoc.createElement(QUaAcDocking::m_strXmlName);
	// lists permissions
	if (m_widgetListPerms)
	{
		elemDock.setAttribute("WidgetListPermissions", m_widgetListPerms->nodeId());
	}
	if (m_layoutListPerms)
	{
		elemDock.setAttribute("LayoutListPermissions", m_layoutListPerms->nodeId());
	}
	// NOTE : only layouts, widgets are serialized by their factories (which might need to serilize extra info)
	for (auto layoutName : this->layoutNames())
	{
		QDomElement elemLayout = domDoc.createElement(QUaAcDocking::m_strXmlLayoutName);
		// set name
		elemLayout.setAttribute("Name", layoutName);
		// set state
		elemLayout.setAttribute("State", QString(m_mapLayouts[layoutName].byteState.toHex()));
		// set permissions if any
		if (m_mapLayouts[layoutName].permsObject)
		{
			elemLayout.setAttribute("Permissions", m_mapLayouts[layoutName].permsObject->nodeId());
		}
		// append
		elemDock.appendChild(elemLayout);
	}
	// return element
	return elemDock;

}

void QUaAcDocking::fromDomElement(QUaAccessControl * ac, QDomElement & domElem, QString & strError)
{
	Q_CHECK_PTR(ac);
	// lists permissions (optional)
	if (domElem.hasAttribute("WidgetListPermissions"))
	{
		auto permissions = this->findPermissions(ac, domElem.attribute("WidgetListPermissions"), strError);
		if (permissions)
		{
			this->setWidgetListPermissions(permissions);
		}
	}
	if (domElem.hasAttribute("LayoutListPermissions"))
	{
		auto permissions = this->findPermissions(ac, domElem.attribute("LayoutListPermissions"), strError);
		if (permissions)
		{
			this->setLayoutListPermissions(permissions);
		}
	}
	// NOTE : only layouts, widgets are serialized by their factories
	QDomNodeList listNodesL = domElem.elementsByTagName(QUaAcDocking::m_strXmlLayoutName);
	for (int i = 0; i < listNodesL.count(); i++)
	{
		QDomElement elem = listNodesL.at(i).toElement();
		Q_ASSERT(!elem.isNull());
		// name is mandatory
		if (!elem.hasAttribute("Name"))
		{
			strError += tr("%1 : %2 element must have Name attribute.")
				.arg("Error")
				.arg(QUaAcDocking::m_strXmlLayoutName);
			continue;
		}
		// state is mandatory
		if (!elem.hasAttribute("State"))
		{
			strError += tr("%1 : %2 element must have State attribute.")
				.arg("Error")
				.arg(QUaAcDocking::m_strXmlLayoutName);
			continue;
		}
		QString strLayoutName = elem.attribute("Name");
		QByteArray byteLayoutState = QByteArray::fromHex(elem.attribute("State").toUtf8());
		// add layout 
		// NOTE : manually
		if (this->hasLayout(strLayoutName))
		{
			m_mapLayouts[strLayoutName].byteState = byteLayoutState;
			emit this->layoutUpdated(strLayoutName);
		}
		else
		{
			m_mapLayouts.insert(strLayoutName, { byteLayoutState, nullptr });
			emit this->layoutAdded(strLayoutName);
		}
		// not having permissions is acceptable
		if (!elem.hasAttribute("Permissions"))
		{
			continue;
		}
		// attempt to add permissions
		QString strNodeId = elem.attribute("Permissions");
		auto permissions  = this->findPermissions(ac, strNodeId, strError);
		if (!permissions)
		{
			continue;
		}
		this->setLayoutPermissions(strLayoutName, permissions);
	}
}

QUaPermissions * QUaAcDocking::findPermissions(QUaAccessControl * ac, const QString & strNodeId, QString & strError)
{
	QUaNode * node = ac->server()->nodeById(strNodeId);
	if (!node)
	{
		strError += tr("%1 : Unexisting node with NodeId %2.")
			.arg("Error")
			.arg(strNodeId);
		return nullptr;
	}
	QUaPermissions * permissions = dynamic_cast<QUaPermissions*>(node);
	if (!permissions)
	{
		strError += tr("%1 : Node with NodeId %2 is not a permissions instance.")
			.arg("Error")
			.arg(strNodeId);
		return nullptr;
	}
	return permissions;
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
	// update layout permissions
	this->updateLayoutPermissions();
	// refresh layout (also updates widget permissions)
	this->setLayout(this->currentLayout());
	// update layout list perms
	this->updateLayoutListPermissions();
	// update widget list perms
	this->updateWidgetListPermissions();
}

void QUaAcDocking::handleWidgetAdded(const QStringList & strWidgetPathName, QMenu * menuParent, const int &index/* = 0*/)
{
	// create menu path if necessary
	if (strWidgetPathName.count()-1 > index)
	{
		// get path part
		QString strPathName = strWidgetPathName.at(index);
		// add if does not exist
		QMenu * menuChild = menuParent->findChild<QMenu*>(strPathName);
		if (!menuChild)
		{
			menuChild = menuParent->addMenu(strPathName);
			menuChild->setObjectName(strPathName);
		}
		Q_CHECK_PTR(menuChild);

		// TODO : model

		this->handleWidgetAdded(strWidgetPathName, menuChild, index + 1);
		// exit
		return;
	}
	// get name
	QString strWidgetName = strWidgetPathName.at(index);
	// add action to menu
	auto widget = m_dockManager->findDockWidget(strWidgetName);
	Q_ASSERT(widget);
	menuParent->addAction(widget->toggleViewAction());
}

void QUaAcDocking::handleWidgetRemoved(const QString & strWidgetName)
{
	/*
	auto widget = m_dockManager->findDockWidget(strWidgetName);
	Q_ASSERT(widget);
	m_widgetsMenu->removeAction(widget->toggleViewAction());
	*/
	// NOTE : find recursivelly by default, uses QObject::objectName
	QAction * action = m_widgetsMenu->findChild<QAction*>(strWidgetName);
	Q_CHECK_PTR(action);
	QMenu * parentMenu = dynamic_cast<QMenu*>(action->parentWidget());
	Q_CHECK_PTR(parentMenu);
	parentMenu->removeAction(action);
}

void QUaAcDocking::on_layoutAdded(const QString & strLayoutName)
{
	QMenu *menuLayouts = m_layoutsMenu->findChild<QMenu*>("Show");
	Q_CHECK_PTR(menuLayouts);
	auto act = menuLayouts->addAction(strLayoutName, this,
	[this, strLayoutName]() {
		this->setLayout(strLayoutName);
	});
	act->setObjectName(strLayoutName);
	act->setCheckable(true);
}

void QUaAcDocking::on_layoutRemoved(const QString & strLayoutName)
{
	QMenu *menuLayouts = m_layoutsMenu->findChild<QMenu*>("Show");
	Q_CHECK_PTR(menuLayouts);
	QAction * layoutAction = menuLayouts->findChild<QAction*>(strLayoutName);
	Q_CHECK_PTR(layoutAction);
	menuLayouts->removeAction(layoutAction);
}
