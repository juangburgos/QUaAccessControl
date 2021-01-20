#include "quapermissionswidgetedit.h"
#include "ui_quapermissionswidgetedit.h"

#include <QMessageBox>
#include <QMetaEnum>

#include <QCheckBox>

#include <QUaAccessControl>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>

QUaPermissionsWidgetEdit::QUaPermissionsWidgetEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaPermissionsWidgetEdit)
{
    ui->setupUi(this);
	m_accessReadOnly = false;
	// forward signals
	QObject::connect(ui->pushButtonDelete, &QPushButton::clicked, this, &QUaPermissionsWidgetEdit::deleteClicked);
	QObject::connect(ui->pushButtonApply , &QPushButton::clicked, this, &QUaPermissionsWidgetEdit::applyClicked );

	// setup roles table model
	m_modelRoles.setColumnCount((int)RoleHeaders::Invalid);
	QStringList roleHeaders;
	for (int i = (int)RoleHeaders::Name; i < (int)RoleHeaders::Invalid; i++)
	{
		roleHeaders << QString(QMetaEnum::fromType<RoleHeaders>().valueToKey(i));
	}
	m_modelRoles.setHorizontalHeaderLabels(roleHeaders);
	// setup role sort filter
	m_proxyRoles.setSourceModel(&m_modelRoles);
	// setup role table
	ui->treeViewRoles->setModel(&m_proxyRoles);
	ui->treeViewRoles->setAlternatingRowColors(true);
	// NOTE : before it was table
	//ui->treeViewRoles->horizontalHeader()->setStretchLastSection(true);
	//ui->treeViewRoles->verticalHeader()->setVisible(false);
	ui->treeViewRoles->setSortingEnabled(true);
	ui->treeViewRoles->sortByColumn((int)RoleHeaders::Name, Qt::SortOrder::AscendingOrder);
	ui->treeViewRoles->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->treeViewRoles->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->treeViewRoles->setEditTriggers(QAbstractItemView::NoEditTriggers);

	// setup users table model
	m_modelUsers.setColumnCount((int)UserHeaders::Invalid);
	QStringList userHeaders;
	for (int i = (int)UserHeaders::Name; i < (int)UserHeaders::Invalid; i++)
	{
		userHeaders << QString(QMetaEnum::fromType<UserHeaders>().valueToKey(i));
	}
	m_modelUsers.setHorizontalHeaderLabels(userHeaders);
	// setup user sort filter
	m_proxyUsers.setSourceModel(&m_modelUsers);
	// setup user table
	ui->treeViewUsers->setModel(&m_proxyUsers);
	ui->treeViewUsers->setAlternatingRowColors(true);
	// NOTE : before it was table
	//ui->treeViewUsers->horizontalHeader()->setStretchLastSection(true);
	//ui->treeViewUsers->verticalHeader()->setVisible(false);
	ui->treeViewUsers->setSortingEnabled(true);
	ui->treeViewUsers->sortByColumn((int)UserHeaders::Name, Qt::SortOrder::AscendingOrder);
	ui->treeViewUsers->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->treeViewUsers->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->treeViewUsers->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

QUaPermissionsWidgetEdit::~QUaPermissionsWidgetEdit()
{
    delete ui;
}

bool QUaPermissionsWidgetEdit::isIdReadOnly() const
{
	return ui->lineEditId->isReadOnly();
}

void QUaPermissionsWidgetEdit::setIdReadOnly(const bool & readOnly)
{
	ui->lineEditId->setReadOnly(readOnly);
}

bool QUaPermissionsWidgetEdit::isIdVisible() const
{
	return ui->framePermsId->isEnabled();
}

void QUaPermissionsWidgetEdit::setIdVisible(const bool & isVisible)
{
	ui->framePermsId->setEnabled(isVisible);
	ui->framePermsId->setVisible(isVisible);
}

bool QUaPermissionsWidgetEdit::isAccessReadOnly() const
{
	return m_accessReadOnly;
}

void QUaPermissionsWidgetEdit::setAccessReadOnly(const bool & readOnly)
{
	m_accessReadOnly = readOnly;
	// since read only, no select
	ui->treeViewRoles->setSelectionMode(m_accessReadOnly ? QAbstractItemView::NoSelection : QAbstractItemView::SingleSelection);
	ui->treeViewUsers->setSelectionMode(m_accessReadOnly ? QAbstractItemView::NoSelection : QAbstractItemView::SingleSelection);
	// TODO : more efficient implementation?
	this->setRoleAccessMap(this->roleAccessMap());
	this->setUserAccessMap(this->userAccessMap());
}

bool QUaPermissionsWidgetEdit::areAccessVisible() const
{
	return ui->treeViewRoles->isEnabled();
}

void QUaPermissionsWidgetEdit::setAccessVisible(const bool & isVisible)
{
	ui->treeViewRoles->setEnabled(isVisible);
	ui->treeViewRoles->setVisible(isVisible);
	ui->labelRoles->setVisible(isVisible);
	ui->treeViewUsers->setEnabled(isVisible);
	ui->treeViewUsers->setVisible(isVisible);
	ui->labelUsers->setVisible(isVisible);
}

bool QUaPermissionsWidgetEdit::areActionsVisible() const
{
	return ui->frameEditActions->isEnabled();
}

void QUaPermissionsWidgetEdit::setActionsVisible(const bool & isVisible)
{
	ui->frameEditActions->setEnabled(isVisible);
	ui->frameEditActions->setVisible(isVisible);
}

QString QUaPermissionsWidgetEdit::id() const
{
	return ui->lineEditId->text();
}

void QUaPermissionsWidgetEdit::setId(const QString & strId)
{
	ui->lineEditId->setText(strId);
}

QUaRoleAccessMap QUaPermissionsWidgetEdit::roleAccessMap() const
{
	QUaRoleAccessMap retMap;
	QWidget   *pWidget = nullptr;
	QCheckBox *pChBox  = nullptr;
	for (int row = 0; row < m_modelRoles.rowCount(); row++)
	{
		// name column
		auto iName = m_modelRoles.item(row, (int)RoleHeaders::Name);
		// read column
		auto iRead = m_modelRoles.item(row, (int)RoleHeaders::Read);
		pWidget = ui->treeViewRoles->indexWidget(m_proxyRoles.mapFromSource(iRead->index()));
		Q_CHECK_PTR(pWidget);
		pChBox = pWidget->findChild<QCheckBox*>("Read");
		Q_CHECK_PTR(pChBox);
		retMap[iName->text()].canRead = pChBox->isChecked();
		// write column
		auto iWrite = m_modelRoles.item(row, (int)RoleHeaders::Write);
		pWidget = ui->treeViewRoles->indexWidget(m_proxyRoles.mapFromSource(iWrite->index()));
		Q_CHECK_PTR(pWidget);
		pChBox = pWidget->findChild<QCheckBox*>("Write");
		Q_CHECK_PTR(pChBox);
		retMap[iName->text()].canWrite = pChBox->isChecked();
	}
	return retMap;
}

void QUaPermissionsWidgetEdit::setRoleAccessMap(const QUaRoleAccessMap & roleMap)
{
	// cleanup first
	m_modelRoles.removeRows(0, m_modelRoles.rowCount());
	QUaRoleAccessIter i(roleMap);
	while (i.hasNext()) 
	{
		i.next();
		this->updateRoleAccess(i.key(), i.value());
	}
}

void QUaPermissionsWidgetEdit::updateRoleAccess(const QString & strRoleName, 
	                                            const QUaRoleAccess & roleAccess)
{
	// check if exists, if not then create
	auto listMatches = m_modelRoles.findItems(strRoleName);
	Q_ASSERT(listMatches.count() <= 1);
	QStandardItem * match = listMatches.count() == 1 ? listMatches.at(0) : nullptr;
	auto parent = m_modelRoles.invisibleRootItem();
	// create new
	QWidget     *pWidget = nullptr;
	QHBoxLayout *pLayout = nullptr;
	QSpacerItem *pSpace1 = nullptr;
	QSpacerItem *pSpace2 = nullptr;
	QCheckBox   *pChBox  = nullptr;
	if (!match)
	{
		auto row = parent->rowCount();
		// name column
		auto iName = new QStandardItem(strRoleName);
		parent->setChild(row, (int)RoleHeaders::Name, iName);

		// read column
		auto iRead = new QStandardItem();
		parent->setChild(row, (int)RoleHeaders::Read, iRead);
		// read checkbox
		pWidget = new QWidget;
		pLayout = new QHBoxLayout;
		pSpace1 = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
		pChBox  = new QCheckBox;
		pSpace2 = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
		pChBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		pChBox->setFocusPolicy(Qt::FocusPolicy::NoFocus);
		pChBox->setChecked(roleAccess.canRead);
		pChBox->setObjectName("Read");
		if (m_accessReadOnly)
		{
			pChBox->setAttribute(Qt::WA_TransparentForMouseEvents);
			pChBox->setFocusPolicy(Qt::NoFocus);
		}
		pLayout->addSpacerItem(pSpace1);
		pLayout->addWidget(pChBox);
		pLayout->addSpacerItem(pSpace2);
		pLayout->setContentsMargins(5, 0, 5, 0);
		pWidget->setLayout(pLayout);
		ui->treeViewRoles->setIndexWidget(m_proxyRoles.mapFromSource(iRead->index()), pWidget);

		// write column
		auto iWrite = new QStandardItem();
		parent->setChild(row, (int)RoleHeaders::Write, iWrite);
		// write checkbox
		pWidget = new QWidget;
		pLayout = new QHBoxLayout;
		pSpace1 = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
		pChBox  = new QCheckBox;
		pSpace2 = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
		pChBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		pChBox->setFocusPolicy(Qt::FocusPolicy::NoFocus);
		pChBox->setChecked(roleAccess.canWrite);
		pChBox->setObjectName("Write");
		if (m_accessReadOnly)
		{
			pChBox->setAttribute(Qt::WA_TransparentForMouseEvents);
			pChBox->setFocusPolicy(Qt::NoFocus);
		}
		pLayout->addSpacerItem(pSpace1);
		pLayout->addWidget(pChBox);
		pLayout->addSpacerItem(pSpace2);
		pLayout->setContentsMargins(5, 0, 5, 0);
		pWidget->setLayout(pLayout);
		ui->treeViewRoles->setIndexWidget(m_proxyRoles.mapFromSource(iWrite->index()), pWidget);

		// exit
		return;
	}
	// update existing
	auto row = match->row();
	// read column
	auto iRead   = m_modelRoles.item(row, (int)RoleHeaders::Read);
	pWidget = ui->treeViewRoles->indexWidget(m_proxyRoles.mapFromSource(iRead->index()));
	Q_CHECK_PTR(pWidget);
	pChBox  = pWidget->findChild<QCheckBox*>("Read");
	Q_CHECK_PTR(pChBox);
	pChBox->setChecked(roleAccess.canRead);
	// write column
	auto iWrite = m_modelRoles.item(row, (int)RoleHeaders::Write);
	pWidget = ui->treeViewRoles->indexWidget(m_proxyRoles.mapFromSource(iWrite->index()));
	Q_CHECK_PTR(pWidget);
	pChBox = pWidget->findChild<QCheckBox*>("Write");
	Q_CHECK_PTR(pChBox);
	pChBox->setChecked(roleAccess.canWrite);
}

QUaUserAccessMap QUaPermissionsWidgetEdit::userAccessMap() const
{
	QUaUserAccessMap retMap;
	QWidget   *pWidget = nullptr;
	QCheckBox *pChBox = nullptr;
	for (int row = 0; row < m_modelUsers.rowCount(); row++)
	{
		// name column
		auto iName = m_modelUsers.item(row, (int)UserHeaders::Name);
		// read column
		auto iRead = m_modelUsers.item(row, (int)UserHeaders::UserRead);
		pWidget = ui->treeViewUsers->indexWidget(m_proxyUsers.mapFromSource(iRead->index()));
		Q_CHECK_PTR(pWidget);
		pChBox = pWidget->findChild<QCheckBox*>("UserRead");
		Q_CHECK_PTR(pChBox);
		retMap[iName->text()].canUserRead = pChBox->isChecked();
		// write column
		auto iWrite = m_modelUsers.item(row, (int)UserHeaders::UserWrite);
		pWidget = ui->treeViewUsers->indexWidget(m_proxyUsers.mapFromSource(iWrite->index()));
		Q_CHECK_PTR(pWidget);
		pChBox = pWidget->findChild<QCheckBox*>("UserWrite");
		Q_CHECK_PTR(pChBox);
		retMap[iName->text()].canUserWrite = pChBox->isChecked();

		// NOTE : ignore role checkboxes because they are read-only here

	}
	return retMap;
}

void QUaPermissionsWidgetEdit::setUserAccessMap(const QUaUserAccessMap & userMap)
{
	// cleanup first
	m_modelUsers.removeRows(0, m_modelUsers.rowCount());
	QUaUserAccessIter i(userMap);
	while (i.hasNext())
	{
		i.next();
		this->updateUserAccess(i.key(), i.value());
	}
}

void QUaPermissionsWidgetEdit::updateUserAccess(const QString & strUserName, 
	                                            const QUaUserAccess & userAccess)
{
	// check if exists, if not then create
	auto listMatches = m_modelUsers.findItems(strUserName);
	Q_ASSERT(listMatches.count() <= 1);
	QStandardItem * match = listMatches.count() == 1 ? listMatches.at(0) : nullptr;
	auto parent = m_modelUsers.invisibleRootItem();
	// create new
	QWidget     *pWidget = nullptr;
	QHBoxLayout *pLayout = nullptr;
	QSpacerItem *pSpace1 = nullptr;
	QSpacerItem *pSpace2 = nullptr;
	QCheckBox   *pChBox  = nullptr;
	if (!match)
	{
		auto row = parent->rowCount();
		// name column
		auto iName = new QStandardItem(strUserName);
		parent->setChild(row, (int)UserHeaders::Name, iName);

		// read user column
		auto iUserRead = new QStandardItem();
		parent->setChild(row, (int)UserHeaders::UserRead, iUserRead);
		// read checkbox
		pWidget = new QWidget;
		pLayout = new QHBoxLayout;
		pSpace1 = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
		pChBox  = new QCheckBox;
		pSpace2 = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
		pChBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		pChBox->setFocusPolicy(Qt::FocusPolicy::NoFocus);
		pChBox->setChecked(userAccess.canUserRead);
		pChBox->setObjectName("UserRead");
		if (m_accessReadOnly)
		{
			pChBox->setAttribute(Qt::WA_TransparentForMouseEvents);
			pChBox->setFocusPolicy(Qt::NoFocus);
		}
		pLayout->addSpacerItem(pSpace1);
		pLayout->addWidget(pChBox);
		pLayout->addSpacerItem(pSpace2);
		pLayout->setContentsMargins(5, 0, 5, 0);
		pWidget->setLayout(pLayout);
		ui->treeViewUsers->setIndexWidget(m_proxyUsers.mapFromSource(iUserRead->index()), pWidget);

		// write user column
		auto iUserWrite = new QStandardItem();
		parent->setChild(row, (int)UserHeaders::UserWrite, iUserWrite);
		// write checkbox
		pWidget = new QWidget;
		pLayout = new QHBoxLayout;
		pSpace1 = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
		pChBox  = new QCheckBox;
		pSpace2 = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
		pChBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		pChBox->setFocusPolicy(Qt::FocusPolicy::NoFocus);
		pChBox->setChecked(userAccess.canUserWrite);
		pChBox->setObjectName("UserWrite");
		if (m_accessReadOnly)
		{
			pChBox->setAttribute(Qt::WA_TransparentForMouseEvents);
			pChBox->setFocusPolicy(Qt::NoFocus);
		}
		pLayout->addSpacerItem(pSpace1);
		pLayout->addWidget(pChBox);
		pLayout->addSpacerItem(pSpace2);
		pLayout->setContentsMargins(5, 0, 5, 0);
		pWidget->setLayout(pLayout);
		ui->treeViewUsers->setIndexWidget(m_proxyUsers.mapFromSource(iUserWrite->index()), pWidget);

		// read role column
		auto iRoleRead = new QStandardItem();
		parent->setChild(row, (int)UserHeaders::RoleRead, iRoleRead);
		// read checkbox
		pWidget = new QWidget;
		pLayout = new QHBoxLayout;
		pSpace1 = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
		pChBox  = new QCheckBox;
		pSpace2 = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
		pChBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		pChBox->setFocusPolicy(Qt::FocusPolicy::NoFocus);
		pChBox->setChecked(userAccess.canRoleRead);
		pChBox->setObjectName("RoleRead");
		// NOTE : always read only (has its own edit table)
		pChBox->setAttribute(Qt::WA_TransparentForMouseEvents);
		pChBox->setFocusPolicy(Qt::NoFocus);
		pLayout->addSpacerItem(pSpace1);
		pLayout->addWidget(pChBox);
		pLayout->addSpacerItem(pSpace2);
		pLayout->setContentsMargins(5, 0, 5, 0);
		pWidget->setLayout(pLayout);
		ui->treeViewUsers->setIndexWidget(m_proxyUsers.mapFromSource(iRoleRead->index()), pWidget);

		// write role column
		auto iRoleWrite = new QStandardItem();
		parent->setChild(row, (int)UserHeaders::RoleWrite, iRoleWrite);
		// write checkbox
		pWidget = new QWidget;
		pLayout = new QHBoxLayout;
		pSpace1 = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
		pChBox  = new QCheckBox;
		pSpace2 = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
		pChBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		pChBox->setFocusPolicy(Qt::FocusPolicy::NoFocus);
		pChBox->setChecked(userAccess.canRoleWrite);
		pChBox->setObjectName("RoleWrite");
		// NOTE : always read only (has its own edit table)
		pChBox->setAttribute(Qt::WA_TransparentForMouseEvents);
		pChBox->setFocusPolicy(Qt::NoFocus);
		pLayout->addSpacerItem(pSpace1);
		pLayout->addWidget(pChBox);
		pLayout->addSpacerItem(pSpace2);
		pLayout->setContentsMargins(5, 0, 5, 0);
		pWidget->setLayout(pLayout);
		ui->treeViewUsers->setIndexWidget(m_proxyUsers.mapFromSource(iRoleWrite->index()), pWidget);

		// exit
		return;
	}
	// update existing
	auto row = match->row();
	// read user column
	auto iUserRead   = m_modelUsers.item(row, (int)UserHeaders::UserRead);
	pWidget = ui->treeViewUsers->indexWidget(m_proxyUsers.mapFromSource(iUserRead->index()));
	Q_CHECK_PTR(pWidget);
	pChBox  = pWidget->findChild<QCheckBox*>("UserRead");
	Q_CHECK_PTR(pChBox);
	pChBox->setChecked(userAccess.canUserRead);
	// write user column
	auto iUserWrite = m_modelUsers.item(row, (int)UserHeaders::UserWrite);
	pWidget = ui->treeViewUsers->indexWidget(m_proxyUsers.mapFromSource(iUserWrite->index()));
	Q_CHECK_PTR(pWidget);
	pChBox = pWidget->findChild<QCheckBox*>("UserWrite");
	Q_CHECK_PTR(pChBox);
	pChBox->setChecked(userAccess.canUserWrite);
	// read role column
	auto iRoleRead = m_modelUsers.item(row, (int)UserHeaders::RoleRead);
	pWidget = ui->treeViewUsers->indexWidget(m_proxyUsers.mapFromSource(iRoleRead->index()));
	Q_CHECK_PTR(pWidget);
	pChBox = pWidget->findChild<QCheckBox*>("RoleRead");
	Q_CHECK_PTR(pChBox);
	pChBox->setChecked(userAccess.canRoleRead);
	// write role column
	auto iRoleWrite = m_modelUsers.item(row, (int)UserHeaders::RoleWrite);
	pWidget = ui->treeViewUsers->indexWidget(m_proxyUsers.mapFromSource(iRoleWrite->index()));
	Q_CHECK_PTR(pWidget);
	pChBox = pWidget->findChild<QCheckBox*>("RoleWrite");
	Q_CHECK_PTR(pChBox);
	pChBox->setChecked(userAccess.canRoleWrite);
}
