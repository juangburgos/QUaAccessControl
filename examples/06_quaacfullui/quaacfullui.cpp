#include "quaacfullui.h"
#include "ui_quaacfullui.h"

#include <QLayout>
#include <QMessageAuthenticationCode>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QInputDialog>
#include <QDockWidget>

#include <QUaAcCommonDialog>
#include <QUaDockWidgetPerms>
#include <QAdDockLayoutBar>

#include <QUaCommonDialog>
#include <QUaLogWidget>

QString QUaAcFullUi::m_strAppName   = QObject::tr("QUaUserAccess GUI");
QString QUaAcFullUi::m_strUntitiled = QObject::tr("Untitled");
QString QUaAcFullUi::m_strDefault   = QObject::tr("Default" );

QString QUaAcFullUi::m_strHelpMenu       = QObject::tr("HelpMenu");
QString QUaAcFullUi::m_strTopDock        = QObject::tr("TopDock");

QUaAcFullUi::QUaAcFullUi(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QUaAcFullUi)
{
    ui->setupUi(this);
	QApplication::setApplicationName(QUaAcFullUi::m_strAppName);
	// initial values
	m_strTitle    = QString("%1 - %2");
	m_deleting    = false;
	m_strSecret   = "my_secret";
	m_loggedUser  = nullptr;
	this->setWindowTitle(m_strTitle.arg(QUaAcFullUi::m_strUntitiled).arg(QUaAcFullUi::m_strAppName));
	// setup opc ua information model and server
	this->setupInfoModel();
	// set permissions model for permissions combo
	this->setupPermsModel();
	// create docking manager NOTE : initially empty set by this->logout();
	m_dockManager = new QUaAcDocking(this, &m_proxyPerms);
	// create and setup access control widgets in dock
	m_acWidgets = new QUaAcDockWidgets<QUaAcFullUi>(this);
	// setup native docks
	this->setupNativeDocks();
	// setup menu bar
	this->setupMenuBar();
	// handle user changed
	QObject::connect(this, &QUaAcFullUi::loggedUserChanged, this         , &QUaAcFullUi::on_loggedUserChanged);
	QObject::connect(this, &QUaAcFullUi::loggedUserChanged, m_dockManager, &QUaAcDocking::on_loggedUserChanged);
	// intially logged out
	this->logout();
}

QUaAcFullUi::~QUaAcFullUi()
{
	m_deleting = true;
    delete ui;
}

QUaAcDocking * QUaAcFullUi::getDockManager() const
{
	return m_dockManager;
}

void QUaAcFullUi::on_loggedUserChanged(QUaUser * user)
{
	// get menu bar widgets
	auto logginBar = dynamic_cast<QMenuBar*>(this->menuBar()->cornerWidget());
	Q_CHECK_PTR(logginBar);
	auto menuHelp = logginBar->findChild<QMenu*>(QUaAcFullUi::m_strHelpMenu);
	Q_CHECK_PTR(menuHelp);
	Q_ASSERT(menuHelp->actions().count() == 1);
	QAction * actLogInOut = menuHelp->actions().at(0);
	// set edit widgets permissions
	m_acWidgets->updateWidgetsPermissions();
	// update ui
	if (!user)
	{
		// update menubar : logged out user
		menuHelp->setTitle(tr("Login Here"));
		actLogInOut->setText(tr("Login"));
		// clear edit widgets
		m_acWidgets->clearWidgets();
		// set empty layout
		m_dockManager->setEmptyLayout();
		return;
	}
	// update menubar : logged in user
	menuHelp->setTitle(user->getName());
	actLogInOut->setText(tr("Logout"));

	// TODO : set user's last state or layout
	m_dockManager->setEmptyLayout();
}

void QUaAcFullUi::on_newConfig()
{
	// clear old config (asks for confirmation is there is a config open)
	if (!this->on_closeConfig())
	{
		return;
	}
	// setup new user widget
	auto widgetNewUser = new QUaUserWidgetEdit;
	widgetNewUser->setActionsVisible(false);
	widgetNewUser->setRoleVisible(false);
	widgetNewUser->setHashVisible(false);
	widgetNewUser->setRepeatVisible(true);
	// setup dialog
	QUaAcCommonDialog dialog(this);
	dialog.setWindowTitle(tr("Create Root User"));
	dialog.setWidget(widgetNewUser);
	// show dialog
	this->showCreateRootUserDialog(dialog);
}

void QUaAcFullUi::on_openConfig()
{
	// setup error dialog just in case
	QMessageBox msgBox;
	msgBox.setWindowTitle(tr("Error"));
	msgBox.setIcon(QMessageBox::Critical);
	// read from file
	QString strConfigFileName = QFileDialog::getOpenFileName(this, tr("Open File"),
		QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
		tr("XML (*.xml)"));
	// validate
	if (strConfigFileName.isEmpty())
	{
		return;
	}
	// compose key file name
	QString strKeyFileName = QString("%1/%2.key").arg(QFileInfo(strConfigFileName).absoluteDir().absolutePath()).arg(QFileInfo(strConfigFileName).baseName());
	// create files
	QFile fileConfig(strConfigFileName);
	QFile fileKey(strKeyFileName);
	// exists
	if (!fileConfig.exists())
	{
		msgBox.setText(tr("File %1 does not exist.").arg(strConfigFileName));
		msgBox.exec();
		return;
	}
	else if (!fileKey.exists())
	{
		msgBox.setText(tr("File %1 does not exist.").arg(strKeyFileName));
		msgBox.exec();
		return;
	}
	else if (fileConfig.open(QIODevice::ReadOnly) && fileKey.open(QIODevice::ReadOnly))
	{
		// load config
		auto byteContents = fileConfig.readAll();
		auto byteKey = fileKey.readAll();
		// compute key
		QByteArray keyComputed = QMessageAuthenticationCode::hash(
			byteContents,
			m_strSecret.toUtf8(),
			QCryptographicHash::Sha512
		).toHex();
		// compare computed key with loaded key
		if (byteKey == keyComputed)
		{
			// close old file
			if (!this->on_closeConfig())
			{
				return;
			}
			// try load config
			auto errorLogs = this->setXmlConfig(byteContents);
			if (!errorLogs.isEmpty())
			{
				// setup log widget
				auto logWidget = new QUaLogWidget;
				logWidget->setFilterVisible(false);
				logWidget->setSettingsVisible(false);
				logWidget->setClearVisible(false);
				logWidget->setColumnVisible(QUaLogWidget::Columns::Timestamp, false);
				logWidget->setColumnVisible(QUaLogWidget::Columns::Category, false);
				logWidget->setLevelColor(QUaLogLevel::Error, QBrush(QColor("#8E2F1C")));
				logWidget->setLevelColor(QUaLogLevel::Warning, QBrush(QColor("#766B0F")));
				logWidget->setLevelColor(QUaLogLevel::Info, QBrush(QColor("#265EB6")));
				bool hasError = false;
				while (errorLogs.count() > 0)
				{
					auto errorLog = errorLogs.dequeue();
					hasError = hasError || errorLog.level == QUaLogLevel::Error ? true : false;
					logWidget->addLog(errorLog);
				}
				// NOTE : dialog takes ownershit
				QUaCommonDialog dialog(this);
				dialog.setWindowTitle(tr("Config Issues"));
				dialog.setWidget(logWidget);
				dialog.clearButtons();
				dialog.addButton(tr("Close"), QDialogButtonBox::ButtonRole::AcceptRole);
				dialog.exec();
				if (hasError)
				{
					this->on_closeConfig();
					return;
				}
			}
			// update title
			this->setWindowTitle(m_strTitle.arg(strConfigFileName).arg(QUaAcFullUi::m_strAppName));
			// login
			this->login();
		}
		else
		{
			msgBox.setText(tr("Corrupted file %1.\nContents do not match key stored in %2.").arg(strConfigFileName).arg(strKeyFileName));
			msgBox.exec();
		}
	}
	else
	{
		msgBox.setText(tr("Files %1, %2 could not be opened.").arg(strConfigFileName).arg(strKeyFileName));
		msgBox.exec();
	}
}

void QUaAcFullUi::on_saveConfig()
{
	// select file
	QString strConfigFileName = QFileDialog::getSaveFileName(this, tr("Save File"),
		QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
		tr("XML (*.xml *.txt)"));
	// ignore if empty
	if (strConfigFileName.isEmpty() || strConfigFileName.isNull())
	{
		return;
	}
	// compose key file name
	QString strKeyFileName = QString("%1/%2.key").arg(QFileInfo(strConfigFileName).absoluteDir().absolutePath()).arg(QFileInfo(strConfigFileName).baseName());
	// save to file
	QFile fileConfig(strConfigFileName);
	QFile fileKey(strKeyFileName);
	if (fileConfig.open(QIODevice::ReadWrite | QFile::Truncate) && fileKey.open(QIODevice::ReadWrite | QFile::Truncate))
	{
		QTextStream streamConfig(&fileConfig);
		QTextStream streamKey(&fileKey);
		// save config in file
		auto byteContents = this->xmlConfig();
		streamConfig << byteContents;
		// create key
		QByteArray key = QMessageAuthenticationCode::hash(
			byteContents,
			m_strSecret.toUtf8(),
			QCryptographicHash::Sha512
		).toHex();
		// save key in file
		streamKey << key;
		// update title
		this->setWindowTitle(m_strTitle.arg(strConfigFileName).arg(QUaAcFullUi::m_strAppName));
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setWindowTitle("Error");
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.setText(tr("Error opening files %1, %2 for write operations.").arg(strConfigFileName).arg(strKeyFileName));
		msgBox.exec();
	}
	// close files
	fileConfig.close();
	fileKey.close();
}

bool QUaAcFullUi::on_closeConfig()
{

	// get access control
	QUaAccessControl * ac = this->accessControl();
	auto listUsers = ac->users()->users();
	// are you sure wanna close?
	if (listUsers.count() > 0)
	{
		// ask user if create new config
		auto res = QMessageBox::question(
			this,
			tr("Confirm Close Config"),
			tr("Closing current configuration will discard any unsaved changes.\nWould you like to close the current configuration?"),
			QMessageBox::StandardButton::Ok,
			QMessageBox::StandardButton::Cancel
		);
		if (res != QMessageBox::StandardButton::Ok)
		{
			return false;
		}
	}
	// close access control widgets
	m_acWidgets->closeConfig();
	// logout
	this->logout();
	// clear config
	// NOTE : need to delete inmediatly, ac->clear(); (deleteLater) won't do the job
	ac->clearInmediatly();
	// clear dock manager (not widgets because some are fixed)
	m_dockManager->setDockListPermissions(nullptr);
	m_dockManager->setLayoutListPermissions(nullptr);
	for (auto dockName : m_dockManager->dockNames())
	{
		m_dockManager->setDockPermissions(dockName, nullptr);
		// TODO : cannot remove widget because they are fixed. Should they be fixed or recreate every time?
	}
	for (auto layoutName : m_dockManager->layoutNames())
	{
		m_dockManager->setLayoutPermissions(layoutName, nullptr);
		m_dockManager->removeLayout(layoutName);
	}
	// update title
	this->setWindowTitle(m_strTitle.arg(QUaAcFullUi::m_strUntitiled).arg(QUaAcFullUi::m_strAppName));
	// success
	return true;
}

void QUaAcFullUi::setupInfoModel()
{
	// setup access control information model
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	auto ac = objsFolder->addChild<QUaAccessControl>("AccessControl");

	// disable anon login
	m_server.setAnonymousLoginAllowed(false);
	// define custom user validation 
	auto listUsers = ac->users();
	m_server.setUserValidationCallback(
	[listUsers](const QString &strUserName, const QString &strPassword) {
		auto user = listUsers->user(strUserName);
		if (!user)
		{
			return false;
		}
		return user->isPasswordValid(strPassword);
	});

	// setup auto add new user permissions
	QObject::connect(ac->users(), &QUaUserList::userAdded,
	[ac](QUaUser * user) {
		// return if user already has permissions
		if (user->hasPermissionsObject())
		{
			return;
		}
		// add user-only permissions
		auto permissionsList = ac->permissions();
		// create instance
		QString strPermId = QString("onlyuser_%1").arg(user->getName());
		permissionsList->addPermissions(strPermId);
		auto permissions = permissionsList->browseChild<QUaPermissions>(strPermId);
		Q_CHECK_PTR(permissions);
		// set user can read write
		permissions->addUserCanWrite(user);
		permissions->addUserCanRead(user);
		// set user's permissions
		user->setPermissionsObject(permissions);
	});
	QObject::connect(ac->users(), &QUaUserList::userRemoved,
	[ac](QUaUser * user) {
		// remove user-only permissions
		auto permissions = user->permissionsObject();
		if (!permissions)
		{
			return;
		}
		permissions->remove();
	});
	// setup auto add new role permissions
	QObject::connect(ac->roles(), &QUaRoleList::roleAdded,
	[ac](QUaRole * role) {
		// return if role already has permissions
		if (role->hasPermissionsObject())
		{
			return;
		}
		// add role-only permissions
		auto permissionsList = ac->permissions();
		// create instance
		QString strPermId = QString("onlyrole_%1").arg(role->getName());
		permissionsList->addPermissions(strPermId);
		auto permissions = permissionsList->browseChild<QUaPermissions>(strPermId);
		Q_CHECK_PTR(permissions);
		// set role can read write
		permissions->addRoleCanWrite(role); // not used
		permissions->addRoleCanRead(role);
		// set role's permissions
		role->setPermissionsObject(permissions);
	});
	QObject::connect(ac->roles(), &QUaRoleList::roleRemoved,
	[ac](QUaRole * role) {
		// remove role-only permissions
		auto permissions = role->permissionsObject();
		if (!permissions)
		{
			return;
		}
		permissions->remove();
	});
	// setup auto add new role permissions
	QObject::connect(ac->permissions(), &QUaPermissionsList::permissionsAdded,
	[ac](QUaPermissions * perms) {
		// return if permissions already has permissions
		if (perms->hasPermissionsObject())
		{
			return;
		}
		// assign to itself
		perms->setPermissionsObject(perms);
	});

	// start server
	m_server.start();
}

void QUaAcFullUi::setupMenuBar()
{
	// file open, save, close
	QMenu *menuFile = this->menuBar()->addMenu(tr("File"));
	menuFile->addAction(tr("New"  ), this, &QUaAcFullUi::on_newConfig);
	menuFile->addAction(tr("Open" ), this, &QUaAcFullUi::on_openConfig);
	menuFile->addSeparator();
	menuFile->addAction(tr("Save" ), this, &QUaAcFullUi::on_saveConfig);
	menuFile->addSeparator();
	menuFile->addAction(tr("Close"), this, &QUaAcFullUi::on_closeConfig);

	// native docks show/hide
	QMenu   *menuView = this->menuBar()->addMenu(tr("View"));
	QAction *actLayoutBar = menuView->addAction(tr("Layout Bar"), this, 
	[this, actLayoutBar](bool checked) {
		QDockWidget *dockTop = this->findChild<QDockWidget*>(QUaAcFullUi::m_strTopDock);
		Q_CHECK_PTR(dockTop);
		if (checked)
		{
			dockTop->show();
		}
		else
		{
			dockTop->hide();
		}
	});
	actLayoutBar->setCheckable(true);
	actLayoutBar->setChecked(true);

	// user widgets
	this->menuBar()->addMenu(m_dockManager->docksMenu());

	// user layouts
	this->menuBar()->addMenu(m_dockManager->layoutsMenu());

	// setup top right loggin menu
	auto logginBar  = new QMenuBar(this->menuBar());
	QMenu *menuHelp = new QMenu(tr("Login"), logginBar);
	menuHelp->setObjectName(QUaAcFullUi::m_strHelpMenu);
	logginBar->addMenu(menuHelp);
	this->menuBar()->setCornerWidget(logginBar);
	// add logon/logout action
	QAction * actLogInOut = menuHelp->addAction(tr("Login"));
	// setup action events
	QObject::connect(actLogInOut, &QAction::triggered, this,
	[this]() {
		if (this->loggedUser())
		{
			this->logout();
		}
		else
		{
			this->login();
		}
	});
}

void QUaAcFullUi::setupNativeDocks()
{
	// top dock - current layout name
	QDockWidget *dockTop = new QDockWidget(this);
	dockTop->setObjectName(QUaAcFullUi::m_strTopDock);
	dockTop->setAllowedAreas(Qt::TopDockWidgetArea);
	dockTop->setFeatures(QDockWidget::NoDockWidgetFeatures);
	dockTop->setFloating(false);
	dockTop->setTitleBarWidget(new QWidget());
	this->addDockWidget(Qt::TopDockWidgetArea, dockTop);
	// widget
	QAdDockLayoutBar *pWidget = new QAdDockLayoutBar(this, &m_proxyPerms, this->m_dockManager->layoutsModel());
	dockTop->setWidget(pWidget);
	// subscribe to user change
	QObject::connect(this, &QUaAcFullUi::loggedUserChanged, pWidget      , &QAdDockLayoutBar::on_loggedUserChanged);
	QObject::connect(this, &QUaAcFullUi::loggedUserChanged, m_dockManager, &QUaAcDocking    ::on_loggedUserChanged);
	// subscribe to layouts changes
	QObject::connect(m_dockManager, &QUaAcDocking::currentLayoutChanged        , pWidget, &QAdDockLayoutBar::on_currentLayoutChanged        );
	QObject::connect(m_dockManager, &QUaAcDocking::layoutPermissionsChanged    , pWidget, &QAdDockLayoutBar::on_layoutPermissionsChanged    );
	QObject::connect(m_dockManager, &QUaAcDocking::layoutListPermissionsChanged, pWidget, &QAdDockLayoutBar::on_layoutListPermissionsChanged);
	// subscribe to bar events
	QObject::connect(pWidget, &QAdDockLayoutBar::setLayout           , m_dockManager, &QUaAcDocking::setLayout           );
	QObject::connect(pWidget, &QAdDockLayoutBar::saveCurrentLayout   , m_dockManager, &QUaAcDocking::saveCurrentLayout   );
	QObject::connect(pWidget, &QAdDockLayoutBar::saveAsCurrentLayout , m_dockManager, &QUaAcDocking::saveAsCurrentLayout );
	QObject::connect(pWidget, &QAdDockLayoutBar::removeCurrentLayout , m_dockManager, &QUaAcDocking::removeCurrentLayout );
	QObject::connect(pWidget, &QAdDockLayoutBar::setLayoutPermissions, m_dockManager, &QUaAcDocking::setLayoutPermissions);
}

QUaAccessControl * QUaAcFullUi::accessControl() const
{
	// get access control
	QUaFolderObject * objsFolder = m_server.objectsFolder();
	QUaAccessControl * ac = objsFolder->browseChild<QUaAccessControl>("AccessControl");
	Q_CHECK_PTR(ac);
	return ac;
}

bool QUaAcFullUi::isDeleting() const
{
	return m_deleting;
}

QUaUser * QUaAcFullUi::loggedUser() const
{
	return m_loggedUser;
}

void QUaAcFullUi::setLoggedUser(QUaUser * user)
{
	if (m_loggedUser == user)
	{
		return;
	}
	m_loggedUser = user;
	emit this->loggedUserChanged(m_loggedUser);
}

void QUaAcFullUi::login()
{
	// get access control
	QUaAccessControl * ac = this->accessControl();
	auto listUsers = ac->users()->users();
	// if no users yet, create root user
	if (listUsers.count() <= 0)
	{
		// ask user if create new config
		auto res = QMessageBox::question(
			this, 
			tr("No Config Loaded"), 
			tr("There is no configuration loaded.\nWould you like to create a new one?"), 
			QMessageBox::StandardButton::Ok,
			QMessageBox::StandardButton::Cancel
		);
		if (res != QMessageBox::StandardButton::Ok)
		{
			return;
		}
		// setup new user widget
		auto widgetNewUser = new QUaUserWidgetEdit;
		widgetNewUser->setActionsVisible(false);
		widgetNewUser->setRoleVisible(false);
		widgetNewUser->setHashVisible(false);
		widgetNewUser->setRepeatVisible(true);
		// setup dialog
		QUaAcCommonDialog dialog(this);
		dialog.setWindowTitle(tr("Create Root User"));
		dialog.setWidget(widgetNewUser);
		// show dialog
		this->showCreateRootUserDialog(dialog);
		return;
	}
	// if users existing, setup widget for credentials
	auto widgetNewUser = new QUaUserWidgetEdit;
	widgetNewUser->setActionsVisible(false);
	widgetNewUser->setRoleVisible(false);
	widgetNewUser->setHashVisible(false);
	// setup dialog
	QUaAcCommonDialog dialog(this);
	dialog.setWindowTitle(tr("Login Credentials"));
	dialog.setWidget(widgetNewUser);
	// show dialog
	this->showUserCredentialsDialog(dialog);
}

void QUaAcFullUi::logout()
{
	// logout
	this->setLoggedUser(nullptr);
	// set empty
	m_dockManager->setEmptyLayout();
}

void QUaAcFullUi::showCreateRootUserDialog(QUaAcCommonDialog & dialog)
{
	int res = dialog.exec();
	if (res != QDialog::Accepted)
	{
		return;
	}
	// get access control
	QUaAccessControl * ac = this->accessControl();
	auto listUsers = ac->users();
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
		QMessageBox::critical(this, tr("Create Root User Error"), tr("Passwords do not match."), QMessageBox::StandardButton::Ok);
		this->showCreateRootUserDialog(dialog);
		return;
	}
	// check
	QString strError = listUsers->addUser(strUserName, strPassword);
	if (strError.contains("Error"))
	{
		QMessageBox::critical(this, tr("Create Root User Error"), strError, QMessageBox::StandardButton::Ok);
		this->showCreateRootUserDialog(dialog);
		return;
	}
	// else set new root user
	QUaUser * root = listUsers->user(strUserName);
	Q_CHECK_PTR(root);
	ac->setRootUser(root);
	// get root permissions object
	auto rootPermissions = root->permissionsObject();
	if (rootPermissions)
	{
		// NOTE : only root can add/remove users, roles and permissions
		ac->setPermissionsObject(rootPermissions);
		ac->users()->setPermissionsObject(rootPermissions);
		ac->roles()->setPermissionsObject(rootPermissions);
		ac->permissions()->setPermissionsObject(rootPermissions);
	}
	else
	{
		QObject::connect(root, &QUaUser::permissionsObjectChanged, this,
		[ac](QUaPermissions * rootPermissions) {
			ac->setPermissionsObject(rootPermissions);
			ac->users()->setPermissionsObject(rootPermissions);
			ac->roles()->setPermissionsObject(rootPermissions);
			ac->permissions()->setPermissionsObject(rootPermissions);
		});
	}
	// login root user
	this->setLoggedUser(root);
}

void QUaAcFullUi::showUserCredentialsDialog(QUaAcCommonDialog & dialog)
{
	int res = dialog.exec();
	if (res != QDialog::Accepted)
	{
		if (!this->loggedUser())
		{
			this->logout();
		}
		return;
	}
	// get access control
	QUaAccessControl * ac = this->accessControl();
	auto listUsers = ac->users();
	// get widget
	auto widgetNewUser = qobject_cast<QUaUserWidgetEdit*>(dialog.widget());
	Q_CHECK_PTR(widgetNewUser);
	// get user data
	QString strUserName = widgetNewUser->userName();
	QString strPassword = widgetNewUser->password();
	// check user
	QUaUser * user = listUsers->user(strUserName);
	if (!user)
	{
		QMessageBox::critical(this, tr("Login Credentials Error"), tr("Username %1 does not exists.").arg(strUserName), QMessageBox::StandardButton::Ok);
		this->showUserCredentialsDialog(dialog);
		return;
	}
	// check pass
	if (!user->isPasswordValid(strPassword))
	{
		QMessageBox::critical(this, tr("Login Credentials Error"), tr("Invalid password for user %1.").arg(strUserName), QMessageBox::StandardButton::Ok);
		this->showUserCredentialsDialog(dialog);
		return;
	}
	// login
	this->setLoggedUser(user);
}

void QUaAcFullUi::setupPermsModel()
{
	// setup combo model
	m_proxyPerms.setSourceModel(&m_modelPerms);
	auto listPerms = this->accessControl()->permissions();
	// add empty/undefined
	auto parent = m_modelPerms.invisibleRootItem();
	auto row = parent->rowCount();
	auto col = 0;
	auto iInvalidParam = new QStandardItem("");
	parent->setChild(row, col, iInvalidParam);
	iInvalidParam->setData(QVariant::fromValue(nullptr), QUaDockWidgetPerms::PointerRole);
	// add all existing perms
	auto perms = listPerms->permissionsList();
	for (auto perm : perms)
	{
		row = parent->rowCount();
		auto iPerm = new QStandardItem(perm->getId());
		parent->setChild(row, col, iPerm);
		iPerm->setData(QVariant::fromValue(perm), QUaDockWidgetPerms::PointerRole);
		// subscribe to destroyed
		QObject::connect(perm, &QObject::destroyed, this,
		[this, iPerm]() {
			Q_CHECK_PTR(iPerm);
			// remove from model
			m_modelPerms.removeRows(iPerm->index().row(), 1);
		});
	}
	// subscribe to perm created
	QObject::connect(listPerms, &QUaPermissionsList::permissionsAdded, this,
	[this, col](QUaPermissions * perm) {
		auto parent = m_modelPerms.invisibleRootItem();
		auto row    = parent->rowCount();
		auto iPerm  = new QStandardItem(perm->getId());
		parent->setChild(row, col, iPerm);
		iPerm->setData(QVariant::fromValue(perm), QUaDockWidgetPerms::PointerRole);
		// subscribe to destroyed
		QObject::connect(perm, &QObject::destroyed, this,
		[this, iPerm]() {
			Q_CHECK_PTR(iPerm);
			if (m_deleting)
			{
				return;
			}
			// remove from model
			m_modelPerms.removeRows(iPerm->index().row(), 1);
		});
	});
}

QByteArray QUaAcFullUi::xmlConfig()
{
	// create dom doc
	QDomDocument doc;
	// set xml header
	QDomProcessingInstruction header = doc.createProcessingInstruction("xml", "version='1.0' encoding='UTF-8'");
	doc.appendChild(header);
	// convert config to xml
	auto elemAc = this->toDomElement(doc);
	doc.appendChild(elemAc);
	// get contents bytearray
	return doc.toByteArray();
}

QQueue<QUaLog> QUaAcFullUi::setXmlConfig(const QByteArray & xmlConfig)
{
	QQueue<QUaLog> errorLogs;
	// set to dom doc
	QDomDocument doc;
	int line, col;
	QString strError;
	doc.setContent(xmlConfig, &strError, &line, &col);
	if (!strError.isEmpty())
	{
		errorLogs << QUaLog(
			tr("Invalid XML in Line %2 Column %3 Error %4.").arg(line).arg(col).arg(strError),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return errorLogs;
	}
	// get list of params
	QDomElement elemApp = doc.firstChildElement(QUaAcFullUi::staticMetaObject.className());
	if (elemApp.isNull())
	{
		errorLogs << QUaLog(
			tr("No Application %1 element found in XML config.").arg(QUaAcFullUi::staticMetaObject.className()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return errorLogs;
	}
	// load config from xml
	this->fromDomElement(elemApp, errorLogs);
	return errorLogs;
}

QDomElement QUaAcFullUi::toDomElement(QDomDocument & domDoc) const
{
	// add element
	QDomElement elemApp = domDoc.createElement(QUaAcFullUi::staticMetaObject.className());
	// access control
	QUaAccessControl * ac = this->accessControl();
	elemApp.appendChild(ac->toDomElement(domDoc));
	// widgets
	elemApp.appendChild(m_acWidgets->toDomElement(domDoc));
	// layouts
	elemApp.appendChild(m_dockManager->toDomElement(domDoc));
	// return ac element
	return elemApp;
}

void QUaAcFullUi::fromDomElement(QDomElement & domElem, QQueue<QUaLog>& errorLogs)
{
	// access control
	QDomElement elemAc = domElem.firstChildElement(QUaAccessControl::staticMetaObject.className());
	if (elemAc.isNull())
	{
		errorLogs << QUaLog(
			tr("No Access Control %1 element found in XML config.").arg(QUaAccessControl::staticMetaObject.className()),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return;
	}
	QUaAccessControl * ac = this->accessControl();
	ac->fromDomElement(elemAc, errorLogs);
	// widgets
	QDomElement elemAcW = domElem.firstChildElement(QUaAcDockWidgets<QUaAcFullUi>::m_strXmlName);
	if (elemAcW.isNull())
	{
		errorLogs << QUaLog(
			tr("No Access Control Widgets %1 element found in XML config.").arg(QUaAcDockWidgets<QUaAcFullUi>::m_strXmlName),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return;
	}
	m_acWidgets->fromDomElement(elemAcW, errorLogs);
	// layouts
	QDomElement elemLayouts = domElem.firstChildElement(QUaAcDocking::m_strXmlName);
	if (elemLayouts.isNull())
	{
		errorLogs << QUaLog(
			tr("No Docking Layouts %1 element found in XML config.").arg(QUaAcDocking::m_strXmlName),
			QUaLogLevel::Error,
			QUaLogCategory::Serialization
		);
		return;
	}
	m_dockManager->fromDomElement(ac, elemLayouts, errorLogs);
}


