#include "qaddocklayoutbar.h"
#include "ui_qaddocklayoutbar.h"

#include <QCompleter>

#include <QUaAcCommonDialog>
#include <QUaDockWidgetPerms>

QAdDockLayoutBar::QAdDockLayoutBar(QWidget *parent, QUaAccessControl *ac) :
    QWidget(parent),
    ui(new Ui::QAdDockLayoutBar)
{
	Q_CHECK_PTR(parent);
	Q_CHECK_PTR(ac);
    ui->setupUi(this);
	// set layouts model for combo
	m_proxyLayouts.setSourceModel(&m_modelLayouts);
	// setup combo
	ui->comboBoxLayout->setModel(&m_modelLayouts);
	ui->comboBoxLayout->setEditable(true);
	// setup completer
	QCompleter *completer = new QCompleter(ui->comboBoxLayout);
	completer->setModel(&m_proxyLayouts);
	completer->setFilterMode(Qt::MatchContains);
	ui->comboBoxLayout->setCompleter(completer);
	ui->comboBoxLayout->setInsertPolicy(QComboBox::NoInsert);
}

QAdDockLayoutBar::~QAdDockLayoutBar()
{
    delete ui;
}

void QAdDockLayoutBar::setLayoutNames(const QList<QString>& listLayoutNames)
{
	auto parent = m_modelLayouts.invisibleRootItem();
	auto row    = parent->rowCount();
	auto col    = 0;
	for (auto layoutName : listLayoutNames)
	{
		row = parent->rowCount();
		auto iLn = new QStandardItem(layoutName);
		parent->setChild(row, col, iLn);
	}
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
	Q_ASSERT(index >= 0);
	// NOTE : setCurrentText does not work for this
	ui->comboBoxLayout->setCurrentIndex(index);
}

void QAdDockLayoutBar::on_layoutPermissionsChanged(const QString & strLayoutName, QUaPermissions * permissions)
{
	// TODO : show/hide actions
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
	// TODO : permissions dialog
}

void QAdDockLayoutBar::on_comboBoxLayout_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	emit this->setLayout(ui->comboBoxLayout->currentText());
}
