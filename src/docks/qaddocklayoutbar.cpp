#include "qaddocklayoutbar.h"
#include "ui_qaddocklayoutbar.h"

#include <QCompleter>
#include <QSignalBlocker>

#include <QUaPermissions>

#include <QUaAcCommonDialog>
#include <QUaDockWidgetPerms>

QAdDockLayoutBar::QAdDockLayoutBar(
	QWidget                * parent, 
	QSortFilterProxyModel  * permsProxy,
	QUaAcLambdaFilterProxy * layoutsProxy
) :
    QWidget(parent),
    ui(new Ui::QAdDockLayoutBar),
	m_proxyPerms(permsProxy),
	m_proxyLayouts(layoutsProxy)
{
	Q_CHECK_PTR(parent);
	Q_CHECK_PTR(permsProxy);
	Q_CHECK_PTR(m_proxyLayouts);
    ui->setupUi(this);
	m_loggedUser      = nullptr;
	m_layoutListPerms = nullptr;
	// setup combo
	ui->comboBoxLayout->setModel(m_proxyLayouts);
	ui->comboBoxLayout->setEditable(true);
	// setup completer
	QCompleter *completer = new QCompleter(ui->comboBoxLayout);
	completer->setModel(m_proxyLayouts);
	completer->setFilterMode(Qt::MatchContains);
	ui->comboBoxLayout->setCompleter(completer);
	ui->comboBoxLayout->setInsertPolicy(QComboBox::NoInsert);
	// tooltips
	ui->pushButtonSave->setToolTip(tr("Save current layout state."));
	ui->pushButtonSaveAs->setToolTip(tr("Save current layout state with a different name."));
	ui->pushButtonRemove->setToolTip(tr("Delete current layout"));
	ui->pushButtonPermissions->setToolTip(tr(
		"Sets this layout's permissions.\n"
		"Read permissions control if the layout is listed in the Layouts menu and in this bar.\n"
		"Write permissions control if the layout can be edited ('Save') or deleted ('Remove')."
	));
}

QAdDockLayoutBar::~QAdDockLayoutBar()
{
    delete ui;
}

void QAdDockLayoutBar::on_loggedUserChanged(QUaUser * user)
{
	m_loggedUser = user;
	// update permissions
	ui->comboBoxLayout->setEnabled(m_loggedUser);
	this->updateLayoutListPermissions();
	this->on_comboBoxLayout_currentIndexChanged(ui->comboBoxLayout->currentIndex());
}

void QAdDockLayoutBar::on_currentLayoutChanged(const QString & strLayoutName)
{
	// check if layout is same
	if (strLayoutName.compare(ui->comboBoxLayout->currentText(), Qt::CaseInsensitive) == 0)
	{
		return;
	}
	// get index of new layout
	auto index = ui->comboBoxLayout->findText(strLayoutName);
	// can happen that current set layout is not visible in proxy model
	if (index < 0)
	{
		ui->frameActions->setVisible(false);
		return;
	}
	// NOTE : setCurrentText does not work for this
	// NOTE : avoid feedback to QAdDockLayoutBar::on_comboBoxLayout_currentIndexChanged
	const QSignalBlocker blocker(ui->comboBoxLayout);
	ui->comboBoxLayout->setCurrentIndex(index);
}

void QAdDockLayoutBar::on_layoutPermissionsChanged(const QString & strLayoutName, QUaPermissions * permissions)
{
	Q_UNUSED(permissions);
	// check if current
	if (ui->comboBoxLayout->currentText().compare(strLayoutName, Qt::CaseInsensitive) != 0)
	{
		return;
	}
	this->on_comboBoxLayout_currentIndexChanged(ui->comboBoxLayout->currentIndex());
}

void QAdDockLayoutBar::on_layoutListPermissionsChanged(QUaPermissions * permissions)
{
	m_layoutListPerms = permissions;
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
	auto newPerms = permsWidget->permissions();
	if (perms == newPerms)
	{
		return;
	}
	// read permissions and set them for widget
	emit this->setLayoutPermissions(strLayoutName, newPerms);
}

void QAdDockLayoutBar::on_comboBoxLayout_currentIndexChanged(int index)
{
	// can happen that current set layout is not visible in proxy model
	if (index < 0)
	{
		return;
	}
	// check if can show actions
	auto perms    = ui->comboBoxLayout->currentData(QUaDockWidgetPerms::PointerRole).value<QUaPermissions*>();
	bool canWrite = perms ? perms->canUserWrite(m_loggedUser) : true;
	this->setLayoutActionsCanWrite(canWrite);
	// ask to change layout
	auto strLayout = ui->comboBoxLayout->currentText();
	emit this->setLayout(strLayout);
}

void QAdDockLayoutBar::updateLayoutListPermissions()
{
	// can read controls if user can create, save, remove or set permissions to individual layouts
	bool canReadList = !m_loggedUser ? false : !m_layoutListPerms ? true : m_layoutListPerms->canUserRead(m_loggedUser);
	ui->frameActions->setVisible(canReadList);
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
