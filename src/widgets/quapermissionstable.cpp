#include "quapermissionstable.h"
#include "ui_quapermissionstable.h"

#include <QMessageBox>
#include <QMetaEnum>
#include <QPainter>

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

	// setup user table
	ui->tableViewPerms->setModel(&m_proxyPerms);
	ui->tableViewPerms->setAlternatingRowColors(true);
	ui->tableViewPerms->horizontalHeader()->setStretchLastSection(true);
	ui->tableViewPerms->verticalHeader()->setVisible(false);
	ui->tableViewPerms->setSortingEnabled(true);
	ui->tableViewPerms->sortByColumn((int)Headers::Id, Qt::SortOrder::AscendingOrder);
	ui->tableViewPerms->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableViewPerms->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableViewPerms->setEditTriggers(QAbstractItemView::NoEditTriggers);
	// setup user table interactions
	QObject::connect(ui->tableViewPerms->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
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
		ui->tableViewPerms->setCurrentIndex(index);
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