#include "quausertable.h"
#include "ui_quausertable.h"

#include <QMessageBox>
#include <QMetaEnum>

#include <QUaAccessControl>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>

#include <QUaAcCommonDialog>
#include <QUaUserWidgetEdit>

int QUaUserTable::PointerRole = Qt::UserRole + 1;

QUaUserTable::QUaUserTable(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaUserTable)
{
    ui->setupUi(this);
	m_ac = nullptr;
	m_loggedUser = nullptr;
	// setup add button
	ui->pushButtonAdd->setEnabled(false); // disable until ac is set
	ui->pushButtonAdd->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	// setup users table model
	m_modelUsers.setColumnCount((int)Headers::Invalid);
	QStringList alarmHeaders;
	for (int i = (int)Headers::Name; i < (int)Headers::Invalid; i++)
	{
		alarmHeaders << QString(QMetaEnum::fromType<Headers>().valueToKey(i));
	}
	m_modelUsers.setHorizontalHeaderLabels(alarmHeaders);
	// setup user sort filter
	m_proxyUsers.setSourceModel(&m_modelUsers);
	m_proxyUsers.setFilterAcceptsRow([this](int sourceRow, const QModelIndex & sourceParent) {
		Q_UNUSED(sourceParent);
		// if no logged user, then no show
		if (!m_loggedUser)
		{
			return false;
		}
		// get any item
		auto iName = m_modelUsers.item(sourceRow, (int)Headers::Name);
		// no item, nothing to do
		if (!iName)
		{
			return false;
		}
		// get user permissions
		auto user  = iName->data(QUaUserTable::PointerRole).value<QUaUser*>();
		Q_CHECK_PTR(user);
		if (!user)
		{
			return false;
		}
		// if no permissions, then full access
		auto perms = user->permissionsObject();
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
	ui->tableViewUsers->setModel(&m_proxyUsers);
	ui->tableViewUsers->setAlternatingRowColors(true);
	ui->tableViewUsers->horizontalHeader()->setStretchLastSection(true);
	ui->tableViewUsers->verticalHeader()->setVisible(false);
	ui->tableViewUsers->setSortingEnabled(true);
	ui->tableViewUsers->sortByColumn((int)Headers::Name, Qt::SortOrder::AscendingOrder);
	ui->tableViewUsers->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableViewUsers->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableViewUsers->setEditTriggers(QAbstractItemView::NoEditTriggers);
	// setup user table interactions
	QObject::connect(ui->tableViewUsers->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
		[this](const QModelIndex &current, const QModelIndex &previous) {
		auto itemPrev = m_modelUsers.itemFromIndex(m_proxyUsers.mapToSource(previous));
		auto itemCurr = m_modelUsers.itemFromIndex(m_proxyUsers.mapToSource(current));
		// previous
		QUaUser * nodePrev = nullptr;
		if (itemPrev)
		{
			nodePrev = itemPrev->data(QUaUserTable::PointerRole).value<QUaUser*>();
		}
		// current
		QUaUser * nodeCurr = nullptr;
		if (itemCurr)
		{
			nodeCurr = itemCurr->data(QUaUserTable::PointerRole).value<QUaUser*>();
		}
		// emit
		emit this->userSelectionChanged(nodePrev, nodeCurr);
	});
}

QUaUserTable::~QUaUserTable()
{
    delete ui;
}

bool QUaUserTable::isAddVisible() const
{
	return ui->pushButtonAdd->isEnabled();
}

void QUaUserTable::setAddVisible(const bool & isVisible)
{
	ui->pushButtonAdd->setEnabled(isVisible);
	ui->pushButtonAdd->setVisible(isVisible);
}

QUaAccessControl * QUaUserTable::accessControl() const
{
	return m_ac;
}

void QUaUserTable::setAccessControl(QUaAccessControl * ac)
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

	// subscribe to user added
	// NOTE : needs to be a queued connection because we want to wait until browseName is set
	QObject::connect(m_ac->users(), &QUaUserList::userAdded, this,
	[this](QUaUser * user) {
		Q_CHECK_PTR(user);
		// add to gui
		auto item = this->handleUserAdded(user);
		// select newly created
		auto index = m_proxyUsers.mapFromSource(item->index());
		ui->tableViewUsers->setCurrentIndex(index);
	}, Qt::QueuedConnection);

	// add already existing users
	auto listUsers = m_ac->users()->users();
	for (int i = 0; i < listUsers.count(); i++)
	{
		auto user = listUsers.at(i);
		Q_CHECK_PTR(user);
		// add to gui
		this->handleUserAdded(user);
	}
}

void QUaUserTable::on_loggedUserChanged(QUaUser * user)
{
	m_loggedUser = user;
	// update table filter
	m_proxyUsers.resetFilter();

	// update add user permissions (from user list)

	// cannot add users if no access control exists
	// or if no user logged in 
	if (!m_ac || !user)
	{
		
		this->setAddVisible(false);
		return;
	}
	auto perms = m_ac->users()->permissionsObject();
	if (!perms)
	{
		this->setAddVisible(true);
		return;
	}
	// if permissions for user list allow to write, then can add users
	this->setAddVisible(perms->canUserWrite(user));
}

void QUaUserTable::on_pushButtonAdd_clicked()
{
	if (!m_ac)
	{
		return;
	}
	// setup widget
	QUaUserWidgetEdit * widgetNewUser = new QUaUserWidgetEdit;
	widgetNewUser->setActionsVisible(false);
	widgetNewUser->setHashVisible(false);
	widgetNewUser->setRoleList(m_ac->roles());
	widgetNewUser->setRepeatVisible(true);
	// NOTE : dialog takes ownershit
	QUaAcCommonDialog dialog(this);
	dialog.setWindowTitle(tr("New User"));
	dialog.setWidget(widgetNewUser);
	// NOTE : call in own method to we can recall it if fails
	this->showNewUserDialog(dialog);
}

void QUaUserTable::showNewUserDialog(QUaAcCommonDialog & dialog)
{
	int res = dialog.exec();
	if (res != QDialog::Accepted)
	{
		return;
	}
	// get widget
	auto widgetNewUser = qobject_cast<QUaUserWidgetEdit*>(dialog.widget());
	Q_CHECK_PTR(widgetNewUser);
	// get user data
	QString strUserName = widgetNewUser->userName().trimmed();
	QString strPassword = widgetNewUser->password().trimmed();
	QString strRepeat   = widgetNewUser->repeat().trimmed();
	// check pass and repeat
	if (strPassword.compare(strRepeat, Qt::CaseSensitive) != 0)
	{
		QMessageBox::critical(this, tr("Create New User Error"), tr("Passwords do not match."), QMessageBox::StandardButton::Ok);
		this->showNewUserDialog(dialog);
		return;
	}
	// check exists
	auto listUsers = m_ac->users();
	QString strError = listUsers->addUser(strUserName, strPassword);
	if (strError.contains("Error"))
	{
		QMessageBox::critical(this, tr("Create New User Error"), strError, QMessageBox::StandardButton::Ok);
		this->showNewUserDialog(dialog);
		return;
	}
	auto user = listUsers->user(strUserName);
	Q_CHECK_PTR(user);
	// get role if any
	auto role = widgetNewUser->role();
	if (!role)
	{
		return;
	}
	user->setRole(role);
}

QStandardItem * QUaUserTable::handleUserAdded(QUaUser * user)
{
	Q_ASSERT_X(user, "QUaUserTable", "User instance must already exist in OPC UA");
	// get parent to add rows as children
	auto parent = m_modelUsers.invisibleRootItem();
	auto row = parent->rowCount();

	// name column
	auto iName = new QStandardItem(user->getName());
	// set data
	iName->setData(QVariant::fromValue(user), QUaUserTable::PointerRole);
	// add after data
	parent->setChild(row, (int)Headers::Name, iName);

	// role column
	QMetaObject::Connection roleDestConn;
	QString strRole = "";
	QUaRole * role = user->role();
	if (role)
	{
		// role name
		strRole = role->getName();
	}
	auto iRole = new QStandardItem(strRole);
	// set data
	iRole->setData(QVariant::fromValue(user), QUaUserTable::PointerRole);
	// add after data
	parent->setChild(row, (int)Headers::Role, iRole);
	// updates
	if (role)
	{
		// subscribe role delete
		roleDestConn = QObject::connect(role, &QObject::destroyed, user,
			[user, iRole]() {
			QString strRole = "";
			if (user->role())
			{
				strRole = user->role()->getName();
			}
			iRole->setText(strRole);
		});
	}
	QObject::connect(user, &QUaUser::roleChanged, this,
	[this, iRole, roleDestConn](QUaRole * role) {
		QString strRole = "";
		if (role)
		{
			strRole = role->getName();
		}
		iRole->setText(strRole);
		// disconnect
		QObject::disconnect(roleDestConn);
	});

	// remove row if deleted
	// NOTE : set this as receiver, so callback is not called if this has been deleted
	QObject::connect(user, &QObject::destroyed, this,
	[this, iName]() {
		Q_CHECK_PTR(iName);
		// remove from table
		m_modelUsers.removeRows(iName->index().row(), 1);
	});

	return iName;
}
