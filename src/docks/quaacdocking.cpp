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
#include <QUaDockWidgetPerms>

const QString QUaAcDocking::m_strEmpty = QObject::tr("Empty");

const QString QUaAcDocking::m_strXmlName       = "QDockManager";
const QString QUaAcDocking::m_strXmlDockName   = "QDockWidget";
const QString QUaAcDocking::m_strXmlLayoutName = "QDockLayout";

QUaAcDocking::QUaAcDocking(QMainWindow           * parent, 
		                   QSortFilterProxyModel * permsFilter)
	: QObject(parent), m_proxyPerms(permsFilter)
{
	Q_CHECK_PTR(parent);
	Q_CHECK_PTR(permsFilter);
	m_loggedUser      = nullptr;
	m_dockListPerms   = nullptr;
	m_layoutListPerms = nullptr;
	m_dockManager     = new QAdDockManager(parent);
	// docks menu
	m_docksMenu = new QMenu(tr("Docks"), m_dockManager);
	m_docksMenu->setToolTipsVisible(true);
	auto actDockListPerms = m_docksMenu->addAction(tr("List Permissions"), 
	[this]() {
		// create permissions widget
		auto permsWidget = new QUaDockWidgetPerms;
		// configure perms widget combo
		permsWidget->setComboModel(m_proxyPerms);
		permsWidget->setPermissions(this->dockListPermissions());
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
		this->setDockListPermissions(permsWidget->permissions());
	});
	actDockListPerms->setObjectName("Permissions");
	actDockListPerms->setToolTip(tr(
		"Read permissions control if 'Docks' menu is shown.\n"
		"Write permissions control if this 'List Permissions' menu is shown."
	));
	m_docksMenu->addSeparator();
	// layouts  menu
	m_layoutsMenu = new QMenu(tr("Layouts"), m_dockManager);
	m_layoutsMenu->setToolTipsVisible(true);
	auto actLayoutListPerms = m_layoutsMenu->addAction(tr("List Permissions"),
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
	});
	actLayoutListPerms->setObjectName("Permissions");
	actLayoutListPerms->setToolTip(tr(
		"Read permissions control if 'Layouts' menu is shown and "
		"if 'Save As' and 'Remove' buttons are shown in the Layout Bar.\n"
		"Write permissions control if this 'List Permissions' menu is shown."
	));
	m_layoutsMenu->addSeparator();
	QMenu *menuLayouts = m_layoutsMenu->addMenu(tr("Show"));
	menuLayouts->setObjectName("Show");
	m_layoutsMenu->addSeparator();
	m_layoutsMenu->addAction(tr("Save")      , this, &QUaAcDocking::saveCurrentLayout  )
		->setObjectName("Save");
	m_layoutsMenu->addAction(tr("Save As..."), this, &QUaAcDocking::saveAsCurrentLayout)
		->setObjectName("SaveAs");
	m_layoutsMenu->addSeparator()->setObjectName("Separator");
	m_layoutsMenu->addAction(tr("Remove")    , this, &QUaAcDocking::removeCurrentLayout)
		->setObjectName("Remove");
	// set empty initially
	this->saveCurrentLayoutInternal(QUaAcDocking::m_strEmpty);
	this->setEmptyLayout();
	//  set layouts model for combos
	this->setupLayoutsModel();
}

QAdDockWidgetArea * QUaAcDocking::addDock(
	const QString     &strDockPathName,
	const QAdDockArea &dockArea,
	QWidget           *widget,
	QWidget           *widgetEdit  /*= nullptr*/,
	QAdWidgetEditFunc  editCallback/*= nullptr*/,
	QAdDockWidgetArea *widgetArea  /*= nullptr*/
)
{
	// get only name from path, name must be unique
	QStringList listPathParts = strDockPathName.split("/");
	QString     strDockName = listPathParts.last();
	// check does not exists
	Q_ASSERT_X(!this->hasDock(strDockName), "addDock", "Repeated Widget Name not supported.");
	if (this->hasDock(strDockName))
	{
		// TODO : remove old, put new?
		return nullptr;
	}
	// create dock
	auto pDock = new QAdDockWidget(strDockName, m_dockManager);
	// create wrapper
	QAdDockWidgetWrapper * wrapper = new QAdDockWidgetWrapper(pDock);
	//wrapper->setTitle(tr("Dock %1").arg(strDockName));
	wrapper->setEditBarVisible(false);
	wrapper->seConfigButtonVisible(widgetEdit ? true : false);
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
	this->handleDockAdded(listPathParts, m_docksMenu);
	// handle config button clicked
	QObject::connect(wrapper, &QAdDockWidgetWrapper::configClicked, pDock,
	[this, strDockName, widgetEdit, editCallback]() {
		Q_CHECK_PTR(widgetEdit);
		// show modal or non-model dialog
		if (editCallback)
		{
			// dialog
			QUaAcCommonDialog dialog(m_dockManager);
			dialog.setWindowTitle(tr("Widget Configuration - %1").arg(strDockName));
			// set and takes ownership
			dialog.setWidget(widgetEdit);
			// exec dialog
			int res = dialog.exec();
			// remove ownership from tab widget (else will be delated when dialog closed)
			widgetEdit->setParent(m_dockManager);
			if (res != QDialog::Accepted)
			{
				return;
			}
			// call apply callback
			editCallback();			
		}
		else
		{
			if (m_mapDialogs.contains(strDockName))
			{
				m_mapDialogs[strDockName]->show();
				m_mapDialogs[strDockName]->raise();
				return;
			}
			// NOTE : get non-modal dialog
			auto dialog = QUaAcCommonDialog::CreateModal(m_dockManager);
			dialog->setWindowTitle(tr("Edit View"));
			// create edit widget and make dialog take ownership
			dialog->setWidget(widgetEdit);
			// NOTE : modify buttons because this dialog edits live
			dialog->clearButtons();
			dialog->addButton(tr("Close"), QDialogButtonBox::ButtonRole::AcceptRole);
			// exec dialog
			dialog->show();
			// NOTE : to avoid opening multiple dialogs
			m_mapDialogs.insert(strDockName, dialog.data());
			QObject::connect(dialog.data(), &QUaAcCommonDialog::dialogDestroyed, this,
			[this, widgetEdit, strDockName]() {
				// remove ownership from tab widget (else will be delated when dialog closed)
				widgetEdit->setParent(m_dockManager);
				// remove from map
				Q_ASSERT(m_mapDialogs.contains(strDockName));
				m_mapDialogs.remove(strDockName);
			});
		}
	});
	// handle permissions button clicked
	QObject::connect(wrapper, &QAdDockWidgetWrapper::permissionsClicked, pDock,
	[this, strDockName]() {
		// create permissions widget
		auto permsWidget = new QUaDockWidgetPerms;
		// configure perms widget combo
		permsWidget->setComboModel(m_proxyPerms);
		permsWidget->setPermissions(this->dockPermissions(strDockName));
		// dialog
		QUaAcCommonDialog dialog(m_dockManager);
		dialog.setWindowTitle(tr("Dock Permissions - %1").arg(strDockName));
		// set and takes ownership
		dialog.setWidget(permsWidget);
		// exec dialog
		int res = dialog.exec();
		if (res != QDialog::Accepted)
		{
			return;
		}
		// read permissions and set them for widget
		this->setDockPermissions(strDockName, permsWidget->permissions());
	});
	// no permissions initially
	this->setDockPermissions(strDockName, nullptr);
	return wAreaNew;
}

void QUaAcDocking::removeDock(const QString & strDockName)
{
	auto dock = m_dockManager->findDockWidget(strDockName);
	if (!dock)
	{
		return;
	}
	this->handleDockRemoved(strDockName);
	m_dockManager->removeDockWidget(dock);
	dock->deleteLater();
}

bool QUaAcDocking::hasDock(const QString & strDockName)
{
	return m_dockManager->dockWidgetsMap().contains(strDockName);
}

QAdDockWidget * QUaAcDocking::dock(const QString &strDockName) const
{
	return m_dockManager->dockWidgetsMap().value(strDockName, nullptr);
}

QList<QString> QUaAcDocking::dockNames() const
{
	return m_dockManager->dockWidgetsMap().keys();
}

bool QUaAcDocking::isDockVisible(const QString & strDockName)
{
	Q_ASSERT_X(this->hasDock(strDockName), "QUaAcDocking", "Dock does not exist.");
	if (!this->hasDock(strDockName))
	{
		return false;
	}
	auto dock = m_dockManager->findDockWidget(strDockName);
	Q_ASSERT_X(dock, "QUaAcDocking", "Invalid dock.");
	if (!dock)
	{
		return false;
	}
	return !dock->isClosed();
}

bool QUaAcDocking::setIsDockVisible(const QString & strDockName, const bool & visible)
{
	Q_ASSERT_X(this->hasDock(strDockName), "QUaAcDocking", "Dock does not exist.");
	if (!this->hasDock(strDockName))
	{
		return false;
	}
	auto dock = m_dockManager->findDockWidget(strDockName);
	Q_ASSERT_X(dock, "QUaAcDocking", "Invalid dock.");
	if (!dock)
	{
		return false;
	}
	// check if already
	if (
		(!dock->isClosed() &&  visible) ||
		( dock->isClosed() && !visible)
	)
	{
		return true;
	}
	dock->toggleView(visible);
	return true;
}

QMenu * QUaAcDocking::docksMenu()
{
	return m_docksMenu;
}

bool QUaAcDocking::hasPermissions(const QString & strDockName) const
{
	return m_mapDockPerms.contains(strDockName);
}

QUaPermissions * QUaAcDocking::dockPermissions(const QString & strDockName) const
{
	return m_mapDockPerms.value(strDockName, nullptr);
}

void QUaAcDocking::setDockPermissions(const QString & strDockName, QUaPermissions * permissions)
{
	if (!permissions)
	{
		m_mapDockPerms.remove(strDockName);
	}
	else
	{
		// set permissions
		m_mapDockPerms[strDockName] = permissions;
	}
	// update permissions
	this->updateDockPermissions(strDockName, permissions);
}

void QUaAcDocking::setDockListPermissions(QUaPermissions * permissions)
{
	m_dockListPerms = permissions;
	this->updateDockListPermissions();
}

QUaPermissions * QUaAcDocking::dockListPermissions() const
{
	return m_dockListPerms;
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

void QUaAcDocking::setupLayoutsModel()
{
	// setup combo model
	m_proxyLayouts.setSourceModel(&m_modelLayouts);
	// NOTE : add existing or subscribe not necessary?
	// setup filter
	m_proxyLayouts.setFilterAcceptsRow(
	[this](int sourceRow, const QModelIndex &sourceParent) {
		Q_UNUSED(sourceParent;)
		auto parent = m_modelLayouts.invisibleRootItem();
		auto iLn    = parent->child(sourceRow);
		if (!iLn)
		{
			return false;
		}
		if (!m_loggedUser)
		{
			return false;
		}
		auto perms = iLn->data(QUaDockWidgetPerms::PointerRole).value<QUaPermissions*>();
		if (!perms)
		{
			return true;
		}
		return perms->canUserRead(m_loggedUser);
	});
}

void QUaAcDocking::saveCurrentLayoutInternal(const QString & strLayoutName)
{
	if (this->hasLayout(strLayoutName))
	{
		m_mapLayouts[strLayoutName].byteState = m_dockManager->saveState();
		this->handleLayoutUpdated(strLayoutName);
	}
	else
	{
		m_mapLayouts.insert(strLayoutName, { m_dockManager->saveState(), nullptr });
		this->handleLayoutAdded(strLayoutName);
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

void QUaAcDocking::updateDockPermissions()
{
	// update dock permissions
	for (auto dockName : this->dockNames())
	{
		this->updateDockPermissions(dockName, m_mapDockPerms.value(dockName, nullptr));
	}
}

void QUaAcDocking::updateLayoutPermissions(const QString & strLayoutName, QUaPermissions * permissions)
{
	// update if visible in menu
	QAction * layActOld = m_layoutsMenu->findChild<QAction*>(strLayoutName);
	Q_CHECK_PTR(layActOld);
	layActOld->setVisible(!m_loggedUser ? false : !permissions ? true : permissions->canUserRead(m_loggedUser));
	// update if visible in model
	auto listItems = m_modelLayouts.findItems(strLayoutName);
	Q_ASSERT(listItems.count() == 1);
	auto iLn = listItems.at(0);
	iLn->setData(QVariant::fromValue(permissions), QUaDockWidgetPerms::PointerRole);
	// check if current
	if (strLayoutName.compare(m_currLayout, Qt::CaseInsensitive) != 0)
	{
		return;
	}
	// update if can edit or remove layout - show/hide layout menu actions
	bool canWrite = !m_loggedUser ? false : !permissions ? true : permissions->canUserWrite(m_loggedUser);
	m_layoutsMenu->findChild<QAction*>("Save"     )->setVisible(canWrite);
	m_layoutsMenu->findChild<QAction*>("Separator")->setVisible(canWrite);
	// NOTE : ability to add or remove layouts goes in conjunction with layout *list* permissions
	bool canReadList = !m_loggedUser ? false : !m_layoutListPerms ? true : m_layoutListPerms->canUserRead(m_loggedUser);
	m_layoutsMenu->findChild<QAction*>("Remove"   )->setVisible(canWrite && canReadList);
	m_layoutsMenu->findChild<QAction*>("SaveAs"   )->setVisible(canReadList);
}

void QUaAcDocking::updateDockPermissions(const QString & strDockName, QUaPermissions * permissions)
{
	// update dock permissions iff current user valid
	auto dock = m_dockManager->findDockWidget(strDockName);
	Q_ASSERT(dock);
	Q_ASSERT(dock->toggleViewAction());
	auto wrapper = dynamic_cast<QAdDockWidgetWrapper*>(dock->widget());
	Q_ASSERT(wrapper);
	// read
	bool canRead = !m_loggedUser ? false : !permissions ? true : permissions->canUserRead(m_loggedUser);
	dock->toggleViewAction()->setVisible(canRead);
	bool isOpen  = !dock->isClosed();
	bool setOpen = !m_loggedUser ? false : !permissions ? isOpen : permissions->canUserRead(m_loggedUser) && isOpen;
	dock->toggleView(setOpen);
	// write (set permissions)
	bool canWrite = !m_loggedUser ? false : !permissions ? true : permissions->canUserWrite(m_loggedUser);
	wrapper->setEditBarVisible(canWrite);
}

void QUaAcDocking::updateLayoutListPermissions()
{
	// NOTE : read permissions affect ability ability to add or remove layouts
	//        in QAdDockLayoutBar, buttons for saving new layouts are also disabled
	// ability to add or remove layouts by show/hide full root menu
	bool canReadList = !m_loggedUser ? false : !m_layoutListPerms ? true : m_layoutListPerms->canUserRead(m_loggedUser);
	m_layoutsMenu->menuAction()->setVisible(canReadList);
	// show/hide layout list permissions action
	bool canWriteList = !m_loggedUser ? false : !m_layoutListPerms ? true : m_layoutListPerms->canUserWrite(m_loggedUser);
	m_layoutsMenu->findChild<QAction*>("Permissions")->setVisible(canWriteList);
}

void QUaAcDocking::updateDockListPermissions()
{
	// show/hide root menu (and that's it! not as complex as layout list)
	bool canRead = !m_loggedUser ? false : !m_dockListPerms ? true : m_dockListPerms->canUserRead(m_loggedUser);
	m_docksMenu->menuAction()->setVisible(canRead);
	// show/hide dock list permissions action
	bool canWrite = !m_loggedUser ? false : !m_dockListPerms ? true : m_dockListPerms->canUserWrite(m_loggedUser);
	m_docksMenu->findChild<QAction*>("Permissions")->setVisible(canWrite);
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
	this->handleLayoutRemoved(strLayoutName);
}

void QUaAcDocking::setLayout(const QString & strLayoutName)
{
	Q_ASSERT(this->hasLayout(strLayoutName));
	if (!this->hasLayout(strLayoutName))
	{
		return;
	}
	// update filter before anything because layout to be set might be filtered out
	m_proxyLayouts.resetFilter();
	// get parent menu
	QMenu *menuLayouts = m_layoutsMenu->findChild<QMenu*>("Show");
	Q_CHECK_PTR(menuLayouts);
	// uncheck old layout action
	QAction * layActOld = menuLayouts->findChild<QAction*>(this->currentLayout());
	Q_CHECK_PTR(layActOld);
	// TODO : sometimes fails, when opening/closing configs, have not been able to reproduce
	if (layActOld) { layActOld->setChecked(false); }
	// update internal (after uncheck old)
	m_currLayout = strLayoutName;
	// emit signal
	emit this->aboutToChangeLayout();
	// set new layout
	m_dockManager->restoreState(m_mapLayouts.value(m_currLayout).byteState);
	// update permissions
	this->updateDockPermissions();
	// check new layout action
	QAction * layActNew = menuLayouts->findChild<QAction*>(m_currLayout);
	Q_CHECK_PTR(layActNew);
	layActNew->setChecked(true);
	// update menu permissions
	this->updateLayoutPermissions(m_currLayout, m_mapLayouts[m_currLayout].permsObject);
	// emit id really changed
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

QUaAcLambdaFilterProxy * QUaAcDocking::layoutsModel()
{
	return &m_proxyLayouts;
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
	if (m_dockListPerms)
	{
		elemDock.setAttribute("DockListPermissions", m_dockListPerms->nodeId());
	}
	if (m_layoutListPerms)
	{
		elemDock.setAttribute("LayoutListPermissions", m_layoutListPerms->nodeId());
	}
	// NOTE : only layouts, docks are serialized by their factories (which might need to serilize extra info)
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
	if (domElem.hasAttribute("DockListPermissions"))
	{
		auto permissions = this->findPermissions(ac, domElem.attribute("DockListPermissions"), strError);
		if (permissions)
		{
			this->setDockListPermissions(permissions);
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
	// NOTE : only layouts, docks are serialized by their factories
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
			this->handleLayoutUpdated(strLayoutName);
		}
		else
		{
			m_mapLayouts.insert(strLayoutName, { byteLayoutState, nullptr });
			this->handleLayoutAdded(strLayoutName);
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
	).trimmed();
	if (!ok && strLayoutName.isEmpty())
	{
		return;
	}

	// TODO : validate text format XML safe ?

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
	if (m_loggedUser == user)
	{
		return;
	}
	m_loggedUser = user;
	// update layout permissions
	this->updateLayoutPermissions();
	// refresh layout (also updates dock permissions)
	this->setLayout(this->currentLayout());
	// update layout list perms
	this->updateLayoutListPermissions();
	// update dock list perms
	this->updateDockListPermissions();
	// update filter for layout combo model
	m_proxyLayouts.resetFilter();
}

void QUaAcDocking::handleDockAdded(const QStringList & strDockPathName, 
	                                 QMenu * menuParent, 
	                                 const int &index/* = 0*/)
{
	// create branch if necessary
	if (strDockPathName.count()-1 > index)
	{
		// get path part
		QString strPathName = strDockPathName.at(index);
		// add branch to menu if does not exist
		QMenu * menuChild = menuParent->findChild<QMenu*>(strPathName);
		if (!menuChild)
		{
			menuChild = menuParent->addMenu(strPathName);
			menuChild->setObjectName(strPathName);
		}
		Q_CHECK_PTR(menuChild);
		this->handleDockAdded(strDockPathName, menuChild, index + 1);
		// exit
		return;
	}
	// create leaf
	QString strDockName = strDockPathName.at(index);
	// add action to menu
	auto dock = m_dockManager->findDockWidget(strDockName);
	Q_CHECK_PTR(dock);
	auto action = dock->toggleViewAction();
	menuParent->addAction(action);
	action->setProperty("ParentMenu", QVariant::fromValue(menuParent));
}

void QUaAcDocking::handleDockRemoved(const QString & strDockName)
{
	auto dock = m_dockManager->findDockWidget(strDockName);
	Q_CHECK_PTR(dock);
	// NOTE : find recursivelly by default, uses QObject::objectName
	QAction * action = dock->toggleViewAction();
	Q_CHECK_PTR(action);
	QMenu * parentMenu = action->property("ParentMenu").value<QMenu*>();
	Q_CHECK_PTR(parentMenu);
	parentMenu->removeAction(action);
}

void QUaAcDocking::handleLayoutAdded(const QString & strLayoutName)
{
	// update menu
	QMenu *menuLayouts = m_layoutsMenu->findChild<QMenu*>("Show");
	Q_CHECK_PTR(menuLayouts);
	Q_ASSERT(!menuLayouts->findChild<QAction*>(strLayoutName));
	auto act = menuLayouts->addAction(strLayoutName, this,
	[this, strLayoutName]() {
		this->setLayout(strLayoutName);
	});
	act->setObjectName(strLayoutName);
	act->setCheckable(true);
	// update model
	auto parent = m_modelLayouts.invisibleRootItem();
	auto iLn    = new QStandardItem(strLayoutName);
	// NOTE : line below fixes random crash about m_proxyLayouts.setFilterAcceptsRow
	iLn->setData(QVariant::fromValue(nullptr), QUaDockWidgetPerms::PointerRole);
	parent->appendRow(iLn);
}

void QUaAcDocking::handleLayoutRemoved(const QString & strLayoutName)
{
	// update menu
	QMenu *menuLayouts = m_layoutsMenu->findChild<QMenu*>("Show");
	Q_CHECK_PTR(menuLayouts);
	QAction * layoutAction = menuLayouts->findChild<QAction*>(strLayoutName);
	Q_CHECK_PTR(layoutAction);
	menuLayouts->removeAction(layoutAction);
	delete layoutAction;
	// update model
	auto listItems = m_modelLayouts.findItems(strLayoutName);
	Q_ASSERT(listItems.count() == 1);
	auto iLn = listItems.at(0);
	// remove from model
	m_modelLayouts.removeRows(iLn->index().row(), 1);
}

void QUaAcDocking::handleLayoutUpdated(const QString & strLayoutName)
{
	// TODO ?
	Q_UNUSED(strLayoutName);
}
