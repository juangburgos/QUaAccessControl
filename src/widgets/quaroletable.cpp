#include "quaroletable.h"
#include "ui_quaroletable.h"

#include <QMessageBox>
#include <QMetaEnum>
#include <QPainter>

#include <QUaAccessControl>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>

#include <QUaAcCommonDialog>
#include <QUaRoleWidgetEdit>

int QUaRoleTable::PointerRole = Qt::UserRole + 1;

QUaRoleTable::QUaRoleTable(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaRoleTable)
{
    ui->setupUi(this);
	m_ac = nullptr;
	m_loggedUser = nullptr;
	// setup add button
	ui->pushButtonAdd->setEnabled(false); // disable until ac is set
	ui->pushButtonAdd->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	// setup users table model
	m_modelRoles.setColumnCount((int)Headers::Invalid);
	QStringList roleHeaders;
	for (int i = (int)Headers::Name; i < (int)Headers::Invalid; i++)
	{
		roleHeaders << QString(QMetaEnum::fromType<Headers>().valueToKey(i));
	}
	m_modelRoles.setHorizontalHeaderLabels(roleHeaders);
	// setup user sort filter
	m_proxyRoles.setSourceModel(&m_modelRoles);
	m_proxyRoles.setFilterAcceptsRow([this](int sourceRow, const QModelIndex & sourceParent) {
		Q_UNUSED(sourceParent);
		// if no logged user, then no show
		if (!m_loggedUser)
		{
			return false;
		}
		// get any item
		auto iName = m_modelRoles.item(sourceRow, (int)Headers::Name);
		// no item, nothing to do
		if (!iName)
		{
			return false;
		}
		// get role permissions
		auto role  = iName->data(QUaRoleTable::PointerRole).value<QUaRole*>();
		Q_CHECK_PTR(role);
		if (!role)
		{
			return false;
		}
		// if no permissions, then full access
		auto perms = role->permissionsObject();
		if (!perms)
		{
			return true;
		}
		// if no can read, then no show
		if (!perms->canUserRead(m_loggedUser))
		{
			return false;
		}
		// return yes can read
		return true;
	});

	// setup user table
	ui->treeViewRoles->setModel(&m_proxyRoles);
	ui->treeViewRoles->setAlternatingRowColors(true);
	// NOTE : before it was table
	//ui->treeViewRoles->horizontalHeader()->setStretchLastSection(true);
	//ui->treeViewRoles->verticalHeader()->setVisible(false);
	ui->treeViewRoles->setSortingEnabled(true);
	ui->treeViewRoles->sortByColumn((int)Headers::Name, Qt::SortOrder::AscendingOrder);
	ui->treeViewRoles->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->treeViewRoles->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->treeViewRoles->setEditTriggers(QAbstractItemView::NoEditTriggers);
	// setup user table interactions
	QObject::connect(ui->treeViewRoles->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
		[this](const QModelIndex &current, const QModelIndex &previous) {
		auto itemPrev = m_modelRoles.itemFromIndex(m_proxyRoles.mapToSource(previous));
		auto itemCurr = m_modelRoles.itemFromIndex(m_proxyRoles.mapToSource(current));
		// previous
		QUaRole * nodePrev = nullptr;
		if (itemPrev)
		{
			nodePrev = itemPrev->data(QUaRoleTable::PointerRole).value<QUaRole*>();
		}
		// current
		QUaRole * nodeCurr = nullptr;
		if (itemCurr)
		{
			nodeCurr = itemCurr->data(QUaRoleTable::PointerRole).value<QUaRole*>();
		}
		// emit
		emit this->roleSelectionChanged(nodePrev, nodeCurr);
	});
}

QUaRoleTable::~QUaRoleTable()
{
    delete ui;
}

bool QUaRoleTable::isAddVisible() const
{
	return ui->pushButtonAdd->isEnabled();
}

void QUaRoleTable::setAddVisible(const bool & isVisible)
{
	ui->pushButtonAdd->setEnabled(isVisible);
	ui->pushButtonAdd->setVisible(isVisible);
}

QUaAccessControl * QUaRoleTable::accessControl() const
{
	return m_ac;
}

void QUaRoleTable::setAccessControl(QUaAccessControl * ac)
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

	// subscribe to role added
	// NOTE : needs to be a queued connection because we want to wait until browseName is set
	QObject::connect(m_ac->roles(), &QUaRoleList::roleAdded, this,
	[this](QUaRole * role) {
		Q_CHECK_PTR(role);
		// add to gui
		auto item = this->handleRoleAdded(role);
		// select newly created
		auto index = m_proxyRoles.mapFromSource(item->index());
		ui->treeViewRoles->setCurrentIndex(index);
	}, Qt::QueuedConnection);

	// add already existing roles
	auto listRoles = m_ac->roles()->roles();
	for (int i = 0; i < listRoles.count(); i++)
	{
		auto role = listRoles.at(i);
		Q_CHECK_PTR(role);
		// add to gui
		this->handleRoleAdded(role);
	}
}

void QUaRoleTable::on_loggedUserChanged(QUaUser * user)
{
	m_loggedUser = user;
	// update table filter
	m_proxyRoles.resetFilter();

	// update add role permissions (from role list)

	// cannot add roles if no access control exists
	// or if no user logged in 
	if (!m_ac || !user)
	{

		this->setAddVisible(false);
		return;
	}
	auto perms = m_ac->roles()->permissionsObject();
	if (!perms)
	{
		this->setAddVisible(true);
		return;
	}
	// if permissions for role list allow to write, then can add users
	this->setAddVisible(perms->canUserWrite(user));
}

void QUaRoleTable::on_pushButtonAdd_clicked()
{
	if (!m_ac)
	{
		return;
	}
	// setup widget
	QUaRoleWidgetEdit * widgetNewRole = new QUaRoleWidgetEdit;
	widgetNewRole->setActionsVisible(false);
	widgetNewRole->setUserListVisible(false);
	// NOTE : dialog takes ownershit
	QUaAcCommonDialog dialog(this);
	dialog.setWindowTitle(tr("New Role"));
	dialog.setWidget(widgetNewRole);
	// NOTE : call in own method to we can recall it if fails
	this->showNewRoleDialog(dialog);
}

void QUaRoleTable::showNewRoleDialog(QUaAcCommonDialog & dialog)
{
	int res = dialog.exec();
	if (res != QDialog::Accepted)
	{
		return;
	}
	// get widget
	auto widgetNewRole = qobject_cast<QUaRoleWidgetEdit*>(dialog.widget());
	Q_CHECK_PTR(widgetNewRole);
	// get role data
	QString strRoleName = widgetNewRole->roleName().trimmed();
	// check
	auto listRoles = m_ac->roles();
	QString strError = listRoles->addRole(strRoleName);
	if (strError.contains("Error"))
	{
		QMessageBox::critical(this, tr("Create New Role Error"), strError, QMessageBox::StandardButton::Ok);
		this->showNewRoleDialog(dialog);
		return;
	}
	Q_CHECK_PTR(listRoles->role(strRoleName));
}

QStandardItem * QUaRoleTable::handleRoleAdded(QUaRole * role)
{
	Q_ASSERT_X(role, "QUaRoleTable", "Role instance must already exist in OPC UA");
	// get parent to add rows as children
	auto parent = m_modelRoles.invisibleRootItem();
	auto row = parent->rowCount();

	// name column
	auto iName = new QStandardItem(role->getName());
	// set data
	iName->setData(QVariant::fromValue(role), QUaRoleTable::PointerRole);
	// add after data
	parent->setChild(row, (int)Headers::Name, iName);

	// remove row if deleted
	// NOTE : set this as receiver, so callback is not called if this has been deleted
	QObject::connect(role, &QObject::destroyed, this,
	[this, iName]() {
		Q_CHECK_PTR(iName);
		// remove from table
		m_modelRoles.removeRows(iName->index().row(), 1);
	});

	return iName;
}
