#include "quausertable.h"
#include "ui_quausertable.h"

#include <QMessageBox>
#include <QMetaEnum>

#include <QUaAccessControl>
#include <QUaUser>
#include <QUaRole>

#include <QUaAcCommonDialog>
#include <QUaUserWidgetEdit>

int QUaUserTable::PointerRole = Qt::UserRole + 1;

QUaUserTable::QUaUserTable(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaUserTable)
{
    ui->setupUi(this);
	m_ac = nullptr;
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

	// add already existing clients
	auto listUsers = m_ac->users()->users();
	for (int i = 0; i < listUsers.count(); i++)
	{
		auto user = listUsers.at(i);
		Q_CHECK_PTR(user);
		// add to gui
		this->handleUserAdded(user);
	}
}

void QUaUserTable::on_pushButtonAdd_clicked()
{
	if (!m_ac)
	{
		return;
	}
	// setup widget
	QUaUserWidgetEdit * widgetNewUser = new QUaUserWidgetEdit;
	widgetNewUser->setHashVisible(false);
	widgetNewUser->setRoleList(m_ac->roles());
	// NOTE : dialog takes ownershit
	QUaAcCommonDialog dialog;
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
	// check
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
	parent->setChild(row, (int)Headers::Name, iName);
	// set data
	iName->setData(QVariant::fromValue(user), QUaUserTable::PointerRole);

	// role column
	QString strRole = "";
	QUaRole * role = user->role();
	if (role)
	{
		strRole = role->getName();
	}
	auto iRole = new QStandardItem(strRole);
	parent->setChild(row, (int)Headers::Role, iRole);
	// set data
	iRole->setData(QVariant::fromValue(user), QUaUserTable::PointerRole);
	// updates
	QObject::connect(user, &QUaUser::roleChanged, this,
	[iRole](QUaRole * role) {
		QString strRole = "";
		if (role)
		{
			strRole = role->getName();
		}
		iRole->setText(strRole);
	});

	// actions column
	auto iActs = new QStandardItem();
	parent->setChild(row, (int)Headers::Actions, iActs);
	// set data
	iActs->setData(QVariant::fromValue(user), QUaUserTable::PointerRole   );
	// widgets
	QWidget     *pWidget = new QWidget;
	QHBoxLayout *pLayout = new QHBoxLayout;
	QSpacerItem *pSpacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
	QPushButton *pButDel = new QPushButton;
	// delete button
	pButDel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pButDel->setText(tr("Delete"));
	pButDel->setObjectName("Delete");
	pButDel->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	QObject::connect(pButDel, &QPushButton::clicked, user,
	[user]() {
		Q_CHECK_PTR(user);
		user->remove();
		// NOTE : removed from tree on &QObject::destroyed callback below
	});
	// layout
	pLayout->addSpacerItem(pSpacer);
	pLayout->addWidget(pButDel);
	pLayout->setContentsMargins(5, 0, 5, 0);
	pWidget->setLayout(pLayout);
	ui->tableViewUsers->setIndexWidget(m_proxyUsers.mapFromSource(iActs->index()), pWidget);

	// ua delete
	// NOTE : set this as receiver, so callback is not called if this has been deleted
	QObject::connect(user, &QObject::destroyed, this,
	[this, iName]() {
		Q_CHECK_PTR(iName);
		// remove from table
		m_modelUsers.removeRows(iName->index().row(), 1);
	});

	return iName;
}
