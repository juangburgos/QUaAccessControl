#include "qaddocklayoutbar.h"
#include "ui_qaddocklayoutbar.h"

#include <QCompleter>

#include <QUaPermissions>

#include <QUaAcCommonDialog>
#include <QUaDockWidgetPerms>

QAdDockLayoutBar::QAdDockLayoutBar(
	QWidget               * parent, 
	QSortFilterProxyModel * permsFilter
) :
    QWidget(parent),
    ui(new Ui::QAdDockLayoutBar),
	m_proxyPerms(permsFilter)
{
	Q_CHECK_PTR(parent);
	Q_CHECK_PTR(permsFilter);
    ui->setupUi(this);
	m_loggedUser = nullptr;
	m_layoutListPerms = nullptr;
	// set layouts model for combo
	m_proxyLayouts.setSourceModel(&m_modelLayouts);
	// setup combo
	ui->comboBoxLayout->setModel(&m_proxyLayouts);
	ui->comboBoxLayout->setEditable(true);
	// setup completer
	QCompleter *completer = new QCompleter(ui->comboBoxLayout);
	completer->setModel(&m_proxyLayouts);
	completer->setFilterMode(Qt::MatchContains);
	ui->comboBoxLayout->setCompleter(completer);
	ui->comboBoxLayout->setInsertPolicy(QComboBox::NoInsert);
	// setup filer
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
		auto perms  = iLn->data(QUaDockWidgetPerms::PointerRole).value<QUaPermissions*>();
		if (!perms)
		{
			return true;
		}
		return perms->canUserRead(m_loggedUser);
	});
}

QAdDockLayoutBar::~QAdDockLayoutBar()
{
    delete ui;
}

void QAdDockLayoutBar::setLayouts(const QUaAcLayouts &mapLayouts)
{
	auto parent = m_modelLayouts.invisibleRootItem();
	auto row    = parent->rowCount();
	auto col    = 0;
	QUaAcLayoutsIter i(mapLayouts);
	while (i.hasNext()) {
		i.next();
		row = parent->rowCount();
		auto iLn = new QStandardItem(i.key());
		parent->setChild(row, col, iLn);
		iLn->setData(QVariant::fromValue(i.value().permsObject), QUaDockWidgetPerms::PointerRole);
	}
	// update permissions
	m_proxyLayouts.resetFilter();
}

void QAdDockLayoutBar::on_layoutAdded(const QString & strLayoutName)
{
	auto parent = m_modelLayouts.invisibleRootItem();
	auto row    = parent->rowCount();
	auto col    = 0;
	auto iLn    = new QStandardItem(strLayoutName);
	parent->setChild(row, col, iLn);
}

void QAdDockLayoutBar::on_layoutRemoved(const QString & strLayoutName)
{
	auto listItems = m_modelLayouts.findItems(strLayoutName);
	Q_ASSERT(listItems.count() == 1);
	auto iLn = listItems.at(0);
	// remove from model
	m_modelLayouts.removeRows(iLn->index().row(), 1);
}

void QAdDockLayoutBar::on_currentLayoutChanged(const QString & strLayoutName)
{
	auto index = ui->comboBoxLayout->findText(strLayoutName);
	// can happen that current set layout is not visible in proxy model
	if (index < 0)
	{
		ui->frameActions->setVisible(false);
		return;
	}
	// NOTE : setCurrentText does not work for this
	ui->comboBoxLayout->setCurrentIndex(index);
}

void QAdDockLayoutBar::on_layoutPermissionsChanged(const QString & strLayoutName, QUaPermissions * permissions)
{
	// set item data
	auto listItems = m_modelLayouts.findItems(strLayoutName);
	Q_ASSERT(listItems.count() == 1);
	auto iLn = listItems.at(0);
	iLn->setData(QVariant::fromValue(permissions), QUaDockWidgetPerms::PointerRole);
	// check if current
	if (ui->comboBoxLayout->currentText().compare(strLayoutName, Qt::CaseInsensitive) == 0)
	{
		// check if can show actions
		bool canWrite = permissions ? permissions->canUserWrite(m_loggedUser) : true;
		this->setLayoutActionsCanWrite(canWrite);
	}
	// update permissions
	m_proxyLayouts.resetFilter();
}

void QAdDockLayoutBar::on_layoutListPermissionsChanged(QUaPermissions * permissions)
{
	m_layoutListPerms = permissions;
	this->updateLayoutListPermissions();
}

void QAdDockLayoutBar::on_loggedUserChanged(QUaUser * user)
{
	m_loggedUser = user;
	// update combo
	ui->comboBoxLayout->setEnabled(m_loggedUser);
	// update permissions
	m_proxyLayouts.resetFilter();
	// update list permissions
	this->updateLayoutListPermissions();
}

void QAdDockLayoutBar::on_pushButtonSave_clicked()
{
	emit this->saveCurrentLayout();
}

void QAdDockLayoutBar::on_pushButtonSaveAs_clicked()
{
	emit this->saveAsCurrentLayout();
}

void QAdDockLayoutBar::on_pushButtonRemove_clicked()
{
	emit this->removeCurrentLayout();
}

void QAdDockLayoutBar::on_pushButtonPermissions_clicked()
{
	// get current layout and permissions
	auto strLayoutName = ui->comboBoxLayout->currentText();
	auto perms = ui->comboBoxLayout->currentData(QUaDockWidgetPerms::PointerRole).value<QUaPermissions*>();
	// create permissions widget
	auto permsWidget = new QUaDockWidgetPerms;
	// configure perms widget combo
	permsWidget->setComboModel(m_proxyPerms);
	permsWidget->setPermissions(perms);
	// dialog
	QUaAcCommonDialog dialog(this);
	dialog.setWindowTitle(tr("Config Layout %1").arg(strLayoutName));
	// takes ownershit
	dialog.setWidget(permsWidget);
	// exec dialog
	int res = dialog.exec();
	if (res != QDialog::Accepted)
	{
		return;
	}
	// read permissions and set them for widget
	emit this->setLayoutPermissions(strLayoutName, permsWidget->permissions());
}

void QAdDockLayoutBar::on_comboBoxLayout_currentIndexChanged(int index)
{
	// can happen that current set layout is not visible in proxy model
	if (index < 0)
	{
		return;
	}
	// check if can show actions
	auto perms = ui->comboBoxLayout->currentData(QUaDockWidgetPerms::PointerRole).value<QUaPermissions*>();
	bool canWrite = perms ? perms->canUserWrite(m_loggedUser) : true;
	this->setLayoutActionsCanWrite(canWrite);
	// ask to change layout
	emit this->setLayout(ui->comboBoxLayout->currentText());
}

void QAdDockLayoutBar::updateLayoutListPermissions()
{
	// can read controls if user can create, save, remove or set permissions to individual layouts
	bool canRead = !m_loggedUser ? false : !m_layoutListPerms ? true : m_layoutListPerms->canUserRead(m_loggedUser);
	ui->frameActions->setVisible(canRead);
	// can write controls if user can set lists permissions, so nothing to do here
}

void QAdDockLayoutBar::setLayoutActionsCanWrite(const bool &canWrite)
{
	ui->pushButtonSave->setVisible(canWrite);
	ui->pushButtonPermissions->setVisible(canWrite);
	ui->pushButtonRemove->setVisible(canWrite);
	ui->line1->setVisible(canWrite);
	ui->line2->setVisible(canWrite);
	// can always save as
	ui->pushButtonSaveAs->setVisible(true);
}
