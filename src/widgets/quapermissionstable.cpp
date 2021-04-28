#include "quapermissionstable.h"
#include "ui_quapermissionstable.h"

#include <QMessageBox>
#include <QMetaEnum>
#include <QPainter>
#include <QMenu>

#include <QUaAccessControl>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>

#include <QUaAcCommonDialog>
#include <QUaPermissionsWidgetEdit>

int QUaPermissionsTable::PointerRole = Qt::UserRole + 1;

QUaPermissionsTable::QUaPermissionsTable(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaPermissionsTable)
{
    ui->setupUi(this);
	m_ac = nullptr;
	m_loggedUser = nullptr;
	// setup add button
	ui->pushButtonAdd->setEnabled(false); // disable until ac is set
	ui->pushButtonAdd->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	// setup users table model
	m_modelPerms.setColumnCount((int)Headers::Invalid);
	QStringList permsHeaders;
	for (int i = (int)Headers::Id; i < (int)Headers::Invalid; i++)
	{
		permsHeaders << QString(QMetaEnum::fromType<Headers>().valueToKey(i));
	}
	m_modelPerms.setHorizontalHeaderLabels(permsHeaders);
	// setup user sort filter
	m_proxyPerms.setSourceModel(&m_modelPerms);
	m_proxyPerms.setFilterAcceptsRow([this](int sourceRow, const QModelIndex & sourceParent) {
		Q_UNUSED(sourceParent);
		// if no logged user, then no show
		if (!m_loggedUser)
		{
			return false;
		}
		// get any item
		auto iId = m_modelPerms.item(sourceRow, (int)Headers::Id);
		// no item, nothing to do
		if (!iId)
		{
			return false;
		}
		// get perms permissions
		auto perms  = iId->data(QUaPermissionsTable::PointerRole).value<QUaPermissions*>();
		Q_CHECK_PTR(perms);
		if (!perms)
		{
			return false;
		}
		// if no permissions, then full access
		auto permsPerms = perms->permissionsObject();
		if (!permsPerms)
		{
			return true;
		}
		// if no can read, then no show
		if (!permsPerms->canUserRead(m_loggedUser))
		{
			return false;
		}
		// return yes can read
		return true;
	});
	// setup perms table
	ui->treeViewPerms->setModel(&m_proxyPerms);
	ui->treeViewPerms->setAlternatingRowColors(true);
	ui->treeViewPerms->setSortingEnabled(true);
	ui->treeViewPerms->sortByColumn((int)Headers::Id, Qt::SortOrder::AscendingOrder);
	ui->treeViewPerms->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->treeViewPerms->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->treeViewPerms->setEditTriggers(QAbstractItemView::NoEditTriggers);
	// setup user table interactions
	QObject::connect(ui->treeViewPerms->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
	[this](const QModelIndex &current, const QModelIndex &previous) {
		auto itemPrev = m_modelPerms.itemFromIndex(m_proxyPerms.mapToSource(previous));
		auto itemCurr = m_modelPerms.itemFromIndex(m_proxyPerms.mapToSource(current));
		// previous
		QUaPermissions * nodePrev = nullptr;
		if (itemPrev)
		{
			nodePrev = itemPrev->data(QUaPermissionsTable::PointerRole).value<QUaPermissions*>();
		}
		// current
		QUaPermissions * nodeCurr = nullptr;
		if (itemCurr)
		{
			nodeCurr = itemCurr->data(QUaPermissionsTable::PointerRole).value<QUaPermissions*>();
		}
		// emit
		emit this->permissionsSelectionChanged(nodePrev, nodeCurr);
	});
	// emit on double click
	QObject::connect(ui->treeViewPerms, &QAbstractItemView::doubleClicked, this,
	[this](const QModelIndex& index) {
		auto item = m_modelPerms.itemFromIndex(m_proxyPerms.mapToSource(index));
		if (!item)
		{
			return;
		}
		auto perms = item->data(QUaPermissionsTable::PointerRole).value<QUaPermissions*>();
		if (!perms)
		{
			return;
		}
		this->permissionsDoubleClicked(perms);
	});
	// context menu
	this->setupTableContextMenu();
}

QUaPermissionsTable::~QUaPermissionsTable()
{
    delete ui;
}

bool QUaPermissionsTable::isAddVisible() const
{
	return ui->pushButtonAdd->isEnabled();
}

void QUaPermissionsTable::setAddVisible(const bool & isVisible)
{
	ui->pushButtonAdd->setEnabled(isVisible);
	ui->pushButtonAdd->setVisible(isVisible);
}

QUaAccessControl * QUaPermissionsTable::accessControl() const
{
	return m_ac;
}

void QUaPermissionsTable::setAccessControl(QUaAccessControl * ac)
{
	// check valid arg
	Q_ASSERT(ac);
	if (!ac) { return; }
	// check not set before
	Q_ASSERT(!m_ac);
	if (m_ac) { return; }
	// set
	m_ac = ac;

	// enable add
	ui->pushButtonAdd->setEnabled(true);

	// subscribe to permissions added
	// NOTE : needs to be a queued connection because we want to wait until browseName is set
	QObject::connect(m_ac->permissions(), &QUaPermissionsList::permissionsAdded, this,
		[this](QUaPermissions * perms) {
		Q_CHECK_PTR(perms);
		// add to gui
		auto item = this->handlePermssionsAdded(perms);
		// select newly created
		auto index = m_proxyPerms.mapFromSource(item->index());
		ui->treeViewPerms->setCurrentIndex(index);
	}, Qt::QueuedConnection);

	// add already existing permissions
	auto listPerms = m_ac->permissions()->permissionsList();
	for (int i = 0; i < listPerms.count(); i++)
	{
		auto perms = listPerms.at(i);
		Q_CHECK_PTR(perms);
		// add to gui
		this->handlePermssionsAdded(perms);
	}
}

void QUaPermissionsTable::on_loggedUserChanged(QUaUser * user)
{
	m_loggedUser = user;
	// update table filter
	m_proxyPerms.resetFilter();

	// update add perms permissions (from perms list)

	// cannot add perms if no access control exists
	// or if no user logged in 
	if (!m_ac || !user)
	{

		this->setAddVisible(false);
		return;
	}
	auto perms = m_ac->permissions()->permissionsObject();
	if (!perms)
	{
		this->setAddVisible(true);
		return;
	}
	// if permissions for role list allow to write, then can add users
	this->setAddVisible(perms->canUserWrite(user));
}

void QUaPermissionsTable::on_pushButtonAdd_clicked()
{
	if (!m_ac)
	{
		return;
	}
	// setup widget
	QUaPermissionsWidgetEdit * widgetNewPerms = new QUaPermissionsWidgetEdit;
	widgetNewPerms->setActionsVisible(false);
	widgetNewPerms->setAccessVisible(false);
	// NOTE : dialog takes ownershit
	QUaAcCommonDialog dialog(this);
	dialog.setWindowTitle(tr("New Permissions Object"));
	dialog.setWidget(widgetNewPerms);
	// NOTE : call in own method to we can recall it if fails
	this->showNewPermissionsDialog(dialog);
}

void QUaPermissionsTable::setupTableContextMenu()
{
	ui->treeViewPerms->setContextMenuPolicy(Qt::CustomContextMenu);
	QObject::connect(ui->treeViewPerms, &QTreeView::customContextMenuRequested, this,
	[this](const QPoint& point) {
		QModelIndex index = ui->treeViewPerms->indexAt(point);
		QMenu contextMenu(ui->treeViewPerms);
		if (!index.isValid())
		{
			contextMenu.addAction(m_iconAdd, tr("Add Permissions"), this,
				[this]() {
					this->on_pushButtonAdd_clicked();
				});
			// exec
			contextMenu.exec(ui->treeViewPerms->viewport()->mapToGlobal(point));
			return;
		}
		auto item = m_modelPerms.itemFromIndex(m_proxyPerms.mapToSource(index));
		if (!item)
		{
			return;
		}
		auto perms = item->data(QUaPermissionsTable::PointerRole).value<QUaPermissions*>();
		if (!perms)
		{
			return;
		}
		// edit user
		contextMenu.addAction(m_iconEdit, tr("Edit"), this,
		[this, perms]() {
			this->permissionsEditClicked(perms);
		});
		contextMenu.addSeparator();
		// delete param
		contextMenu.addAction(m_iconDelete, tr("Delete"), this,
		[this, perms]() {
			// are you sure?
			auto res = QMessageBox::question(
				this,
				tr("Delete Permissions Confirmation"),
				tr("Would you like to delete permissions %1?").arg(perms->getId()),
				QMessageBox::StandardButton::Ok,
				QMessageBox::StandardButton::Cancel
			);
			if (res != QMessageBox::StandardButton::Ok)
			{
				return;
			}
			// delete
			perms->remove();
		});
		// exec
		contextMenu.exec(ui->treeViewPerms->viewport()->mapToGlobal(point));
	});
}

void QUaPermissionsTable::showNewPermissionsDialog(QUaAcCommonDialog & dialog)
{
	int res = dialog.exec();
	if (res != QDialog::Accepted)
	{
		return;
	}
	// get widget
	auto widgetNewPerms = qobject_cast<QUaPermissionsWidgetEdit*>(dialog.widget());
	Q_CHECK_PTR(widgetNewPerms);
	// get permissions data
	QString strId = widgetNewPerms->id().trimmed();
	// check
	auto listPerms = m_ac->permissions();
	QString strError = listPerms->addPermissions(strId);
	if (strError.contains("Error"))
	{
		QMessageBox::critical(this, tr("Create New Permissions Object Error"), strError, QMessageBox::StandardButton::Ok);
		this->showNewPermissionsDialog(dialog);
		return;
	}
	Q_CHECK_PTR(listPerms->permission(strId));
}

QStandardItem * QUaPermissionsTable::handlePermssionsAdded(QUaPermissions * perms)
{
	Q_ASSERT_X(perms, "QUaPermissionsTable", "Permissions instance must already exist in OPC UA");
	// get parent to add rows as children
	auto parent = m_modelPerms.invisibleRootItem();
	auto row = parent->rowCount();

	// id column
	auto iId = new QStandardItem(perms->getId());
	// set data
	iId->setData(QVariant::fromValue(perms), QUaPermissionsTable::PointerRole);
	// add after data
	parent->setChild(row, (int)Headers::Id, iId);

	// remove row if deleted
	// NOTE : set this as receiver, so callback is not called if this has been deleted
	QObject::connect(perms, &QObject::destroyed, this,
	[this, iId]() {
		Q_CHECK_PTR(iId);
		// remove from table
		m_modelPerms.removeRows(iId->index().row(), 1);
	});

	return iId;
}

QIcon QUaPermissionsTable::iconAdd() const
{
	return m_iconAdd;
}

void QUaPermissionsTable::setIconAdd(const QIcon& icon)
{
	m_iconAdd = icon;
}

QIcon QUaPermissionsTable::iconEdit() const
{
	return m_iconEdit;
}

void QUaPermissionsTable::setIconEdit(const QIcon& icon)
{
	m_iconEdit = icon;
}

QIcon QUaPermissionsTable::iconDelete() const
{
	return m_iconDelete;
}

void QUaPermissionsTable::setIconDelete(const QIcon& icon)
{
	m_iconDelete = icon;
}

QIcon QUaPermissionsTable::iconClear() const
{
	return m_iconClear;
}

void QUaPermissionsTable::setIconClear(const QIcon& icon)
{
	m_iconClear = icon;
}

QByteArray QUaPermissionsTable::headerState() const
{
	auto header = ui->treeViewPerms->header();
	return header->saveState();
}

void QUaPermissionsTable::setHeaderState(const QByteArray& state)
{
	auto header = ui->treeViewPerms->header();
	header->restoreState(state);
}
