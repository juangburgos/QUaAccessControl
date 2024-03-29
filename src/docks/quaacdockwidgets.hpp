#ifndef QUAACDOCKWIDGETS_H
#define QUAACDOCKWIDGETS_H

#include <QUaAcDocking>

// NOTE : might be necessary to forward declare and move to T.cpp
#include <QUaAccessControl>
#include <QUaUser>
#include <QUaRole>
#include <QUaPermissions>
// NOTE : might be necessary to forward declare and move to T.cpp
#include <QUaUserTable>
#include <QUaRoleTable>
#include <QUaPermissionsTable>
#include <QUaUserWidgetEdit>
#include <QUaRoleWidgetEdit>
#include <QUaPermissionsWidgetEdit>

#include <QMessageBox>
#include <QSettings>

template <class T>
class QUaAcDockWidgets : public QObject
{
public:
    explicit QUaAcDockWidgets(T *parent = nullptr);

	// NOTE : all public methods are T requirements

	void updateWidgetsPermissions();
	void clearWidgets();
	void closeConfig();
	void showDefaultWidgets();

	void loadSettings(const QSettings &settings);
	void saveSettings(QSettings& settings);

	// XML import / export
	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QQueue<QUaLog>& errorLogs);

	const static QString m_strXmlName;

private:
	T * m_thiz;

	// ac widgets
	QUaUserTable             * m_userTable  ;
	QUaRoleTable             * m_roleTable  ;
	QUaPermissionsTable      * m_permsTable ;
	QUaUserWidgetEdit        * m_userWidget ;
	QUaRoleWidgetEdit        * m_roleWidget ;
	QUaPermissionsWidgetEdit * m_permsWidget;

	void createAcWidgetsDocks();
	void setupUserWidgets    ();
	void setupRoleWidgets    ();
	void setupPermsWidgets   ();
	
	QList<QMetaObject::Connection> m_connsUserWidget;
	void clearWidgetUserEdit();
	void bindWidgetUserEdit(QUaUser * user);
	void updateWidgetUserEditPermissions(QUaUser * user);

	QList<QMetaObject::Connection> m_connsRoleWidget;
	void clearWidgetRoleEdit();
	void bindWidgetRoleEdit(QUaRole * role);
	void updateWidgetRoleEditPermissions();

	QList<QMetaObject::Connection> m_connsPermsWidget;
	void clearWidgetPermissionsEdit();
	void bindWidgetPermissionsEdit(QUaPermissions * perms);
	void updateWidgetPermissionsEditPermissions();

	// helpers
	QUaAcDocking * getDockManager() const;
	void showDock(const QString& strDockName);

	const static QString m_strMenuPath;
	const static QString m_strUsersTable;
	const static QString m_strRolesTable;
	const static QString m_strPermissionsTable;
	const static QString m_strUserEdit;
	const static QString m_strRoleEdit;
	const static QString m_strPermissionsEdit;
	static QList<QString> m_listWidgetNames;
};

template <class T>
const QString QUaAcDockWidgets<T>::m_strXmlName = "QUaAcDockWidgets";

template <class T>
const QString QUaAcDockWidgets<T>::m_strMenuPath = "Permissions";

template <class T>
const QString QUaAcDockWidgets<T>::m_strUsersTable = "Users Table";

template <class T>
const QString QUaAcDockWidgets<T>::m_strRolesTable = "Roles Table";

template <class T>
const QString QUaAcDockWidgets<T>::m_strPermissionsTable = "Permissions Table";

template <class T>
const QString QUaAcDockWidgets<T>::m_strUserEdit = "User Edit";

template <class T>
const QString QUaAcDockWidgets<T>::m_strRoleEdit = "Role Edit";

template <class T>
const QString QUaAcDockWidgets<T>::m_strPermissionsEdit = "Permissions Edit";

// create list to iterate
template <class T>
QList<QString> QUaAcDockWidgets<T>::m_listWidgetNames;

template <class T>
inline QUaAcDockWidgets<T>::QUaAcDockWidgets(T *parent) : QObject(parent)
{
	Q_CHECK_PTR(parent);
	m_thiz = parent;
	m_userTable   = nullptr;
	m_roleTable   = nullptr;
	m_permsTable  = nullptr;
	m_userWidget  = nullptr;
	m_roleWidget  = nullptr;
	m_permsWidget = nullptr;
	//
	QUaAcDockWidgets<T>::m_listWidgetNames = QList<QString>()
		<< QUaAcDockWidgets<T>::m_strUsersTable
		<< QUaAcDockWidgets<T>::m_strRolesTable
		<< QUaAcDockWidgets<T>::m_strPermissionsTable
		<< QUaAcDockWidgets<T>::m_strUserEdit
		<< QUaAcDockWidgets<T>::m_strRoleEdit
		<< QUaAcDockWidgets<T>::m_strPermissionsEdit;
	// create access control widgets
	this->createAcWidgetsDocks();
	// setup widgets
	this->setupUserWidgets();
	this->setupRoleWidgets();
	this->setupPermsWidgets();
}

template<class T>
inline void QUaAcDockWidgets<T>::updateWidgetsPermissions()
{
	this->updateWidgetUserEditPermissions(nullptr);
	this->updateWidgetRoleEditPermissions();
	this->updateWidgetPermissionsEditPermissions();
}

template<class T>
inline void QUaAcDockWidgets<T>::clearWidgets()
{
	this->clearWidgetUserEdit();
	this->clearWidgetRoleEdit();
	this->clearWidgetPermissionsEdit();
}

template<class T>
inline void QUaAcDockWidgets<T>::closeConfig()
{
	// disable old connections
	while (m_connsUserWidget.count() > 0)
	{
		QObject::disconnect(m_connsUserWidget.takeFirst());
	}
	while (m_connsRoleWidget.count() > 0)
	{
		QObject::disconnect(m_connsRoleWidget.takeFirst());
	}
	while (m_connsPermsWidget.count() > 0)
	{
		QObject::disconnect(m_connsPermsWidget.takeFirst());
	}
	// clear user edit widget
	this->clearWidgetUserEdit();
	this->clearWidgetRoleEdit();
	this->clearWidgetPermissionsEdit();
}

template<class T>
inline void QUaAcDockWidgets<T>::showDefaultWidgets()
{
	// TODO
}

template<class T>
inline void QUaAcDockWidgets<T>::loadSettings(const QSettings& settings)
{
	if (settings.contains("QUaUserTable::headerState"))
	{
		m_userTable->setHeaderState(settings.value("QUaUserTable::headerState").toByteArray());
	}
	if (settings.contains("QUaRoleTable::headerState"))
	{
		m_roleTable->setHeaderState(settings.value("QUaRoleTable::headerState").toByteArray());
	}
	if (settings.contains("QUaPermissionsTable::headerState"))
	{
		m_permsTable->setHeaderState(settings.value("QUaPermissionsTable::headerState").toByteArray());
	}
	if (settings.contains("QUaPermissionsWidgetEdit::rolesHeaderState"))
	{
		m_permsWidget->setRolesHeaderState(settings.value("QUaPermissionsWidgetEdit::rolesHeaderState").toByteArray());
	}
	if (settings.contains("QUaPermissionsWidgetEdit::usersHeaderState"))
	{
		m_permsWidget->setUsersHeaderState(settings.value("QUaPermissionsWidgetEdit::usersHeaderState").toByteArray());
	}
}

template<class T>
inline void QUaAcDockWidgets<T>::saveSettings(QSettings& settings)
{
	settings.setValue("QUaUserTable::headerState", m_userTable->headerState());
	settings.setValue("QUaRoleTable::headerState", m_roleTable->headerState());
	settings.setValue("QUaPermissionsTable::headerState", m_permsTable->headerState());
	settings.setValue("QUaPermissionsWidgetEdit::rolesHeaderState", m_permsWidget->rolesHeaderState());
	settings.setValue("QUaPermissionsWidgetEdit::usersHeaderState", m_permsWidget->usersHeaderState());
}

template<class T>
inline QDomElement QUaAcDockWidgets<T>::toDomElement(QDomDocument & domDoc) const
{
	// add element
	QDomElement elemAcDockW = domDoc.createElement(QUaAcDockWidgets<T>::m_strXmlName);
	// serialize each widget
	for (auto wName : QUaAcDockWidgets<T>::m_listWidgetNames)
	{
		QDomElement elemW = domDoc.createElement(QUaAcDocking::m_strXmlDockName);
		// set name
		elemW.setAttribute("Name", wName);
		// set permissions if any
		QUaPermissions * perms = this->getDockManager()->dockPermissions(wName);
		if (perms)
		{
			elemW.setAttribute("Permissions", perms->nodeId());
		}
		// append
		elemAcDockW.appendChild(elemW);
	}
	// return element
	return elemAcDockW;
}

template<class T>
inline void QUaAcDockWidgets<T>::fromDomElement(QDomElement & domElem, QQueue<QUaLog>& errorLogs)
{
	QDomNodeList listNodesW = domElem.elementsByTagName(QUaAcDocking::m_strXmlDockName);
	for (int i = 0; i < listNodesW.count(); i++)
	{
		QDomElement elem = listNodesW.at(i).toElement();
		Q_ASSERT(!elem.isNull());
		Q_ASSERT(QUaAcDockWidgets<T>::m_listWidgetNames.contains(elem.attribute("Name")));
		// not having permissions is acceptable
		if (!elem.hasAttribute("Permissions"))
		{
			continue;
		}
		// attempt to add permissions
		QString strPermissionsNodeId = elem.attribute("Permissions");
		QUaNode * node = m_thiz->accessControl()->server()->nodeById(strPermissionsNodeId);
		if (!node)
		{
			errorLogs << QUaLog(
				tr("Unexisting node with NodeId %1.").arg(strPermissionsNodeId),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		QUaPermissions * permissions = qobject_cast<QUaPermissions*>(node);
		if (!permissions)
		{
			errorLogs << QUaLog(
				tr("Node with NodeId %1 is not a permissions instance.").arg(strPermissionsNodeId),
				QUaLogLevel::Error,
				QUaLogCategory::Serialization
			);
			continue;
		}
		QString strWidgetName = elem.attribute("Name");
		if (!this->getDockManager()->hasDock(strWidgetName))
		{
			errorLogs << QUaLog(
				tr("Dock %1 does not exist for permissions instance with NodeId %2. Ignoring.").arg(strWidgetName).arg(strPermissionsNodeId),
				QUaLogLevel::Warning,
				QUaLogCategory::Serialization
			);
			continue;
		}
		this->getDockManager()->setDockPermissions(strWidgetName, permissions);
	}
}

template<class T>
inline void QUaAcDockWidgets<T>::createAcWidgetsDocks()
{
	m_userTable = new QUaUserTable(m_thiz);
	this->getDockManager()->addDock(
		QUaAcDockWidgets<T>::m_strMenuPath + "/" + QUaAcDockWidgets<T>::m_strUsersTable,
		QAd::CenterDockWidgetArea,
		m_userTable
	);
	QObject::connect(m_userTable, &QUaUserTable::showRolesClicked, this,
	[this]() {
		this->showDock(QUaAcDockWidgets<T>::m_strRolesTable);
	});

	m_roleTable = new QUaRoleTable(m_thiz);
	this->getDockManager()->addDock(
		QUaAcDockWidgets<T>::m_strMenuPath + "/" + QUaAcDockWidgets<T>::m_strRolesTable,
		QAd::CenterDockWidgetArea,
		m_roleTable
	);

	m_permsTable = new QUaPermissionsTable(m_thiz);
	this->getDockManager()->addDock(
		QUaAcDockWidgets<T>::m_strMenuPath + "/" + QUaAcDockWidgets<T>::m_strPermissionsTable,
		QAd::CenterDockWidgetArea,
		m_permsTable
	);

	m_userWidget = new QUaUserWidgetEdit(m_thiz);
	this->getDockManager()->addDock(
		QUaAcDockWidgets<T>::m_strMenuPath + "/" + QUaAcDockWidgets<T>::m_strUserEdit,
		QAd::CenterDockWidgetArea,
		m_userWidget
	);
	QObject::connect(m_userWidget, &QUaUserWidgetEdit::showRolesClicked, this,
	[this]() {
		this->showDock(QUaAcDockWidgets<T>::m_strRolesTable);
	});

	m_roleWidget = new QUaRoleWidgetEdit(m_thiz);
	this->getDockManager()->addDock(
		QUaAcDockWidgets<T>::m_strMenuPath + "/" + QUaAcDockWidgets<T>::m_strRoleEdit,
		QAd::CenterDockWidgetArea,
		m_roleWidget
	);

	m_permsWidget = new QUaPermissionsWidgetEdit(m_thiz);
	this->getDockManager()->addDock(
		QUaAcDockWidgets<T>::m_strMenuPath + "/" + QUaAcDockWidgets<T>::m_strPermissionsEdit,
		QAd::CenterDockWidgetArea,
		m_permsWidget
	);
	QObject::connect(m_permsWidget, &QUaPermissionsWidgetEdit::showPermsClicked, this,
	[this]() {
		this->showDock(QUaAcDockWidgets<T>::m_strPermissionsTable);
	});
}

template<class T>
inline void QUaAcDockWidgets<T>::setupUserWidgets()
{
	Q_CHECK_PTR(m_userTable);
	Q_CHECK_PTR(m_userWidget);
	// disable until some valid object selected
	m_userWidget->setEnabled(false);
	m_userWidget->setRepeatVisible(true);
	// get access control
	QUaAccessControl * ac = m_thiz->accessControl();
	// set ac to table
	QObject::connect(m_thiz, &T::loggedUserChanged, m_userTable, &QUaUserTable::on_loggedUserChanged);
	m_userTable->setAccessControl(ac);
	// handle table events (user selection)
	// change widgets
	QObject::connect(m_userTable, &QUaUserTable::userSelectionChanged, this,
	[this](QUaUser * userPrev, QUaUser * userCurr)
	{
		Q_UNUSED(userPrev);
		// early exit
		if (!userCurr || m_thiz->isDeleting())
		{
			return;
		}
		// bind widget for current selection
		this->bindWidgetUserEdit(userCurr);
	});
	// open user edit widget when double clicked
	QObject::connect(m_userTable, &QUaUserTable::userDoubleClicked, this,
	[this](QUaUser* user) {
		Q_UNUSED(user);
		this->showDock(QUaAcDockWidgets<T>::m_strUserEdit);
	});
	QObject::connect(m_userTable, &QUaUserTable::userEditClicked, this,
	[this](QUaUser* user) {
		Q_UNUSED(user);
		this->showDock(QUaAcDockWidgets<T>::m_strUserEdit);
	});
	// setup user edit widget
	m_userWidget->setRoleList(ac->roles());
}

template<class T>
inline void QUaAcDockWidgets<T>::setupRoleWidgets()
{
	Q_CHECK_PTR(m_roleTable);
	Q_CHECK_PTR(m_roleWidget);
	// disable until some valid object selected
	m_roleWidget->setEnabled(false);
	// get access control
	QUaAccessControl * ac = m_thiz->accessControl();
	// set ac to table
	QObject::connect(m_thiz, &T::loggedUserChanged, m_roleTable, &QUaRoleTable::on_loggedUserChanged);
	m_roleTable->setAccessControl(ac);
	// handle table events (user selection)
	// change widgets
	QObject::connect(m_roleTable, &QUaRoleTable::roleSelectionChanged, this,
	[this](QUaRole * rolePrev, QUaRole * roleCurr)
	{
		Q_UNUSED(rolePrev);
		// early exit
		if (!roleCurr || m_thiz->isDeleting())
		{
			return;
		}
		// bind widget for current selection
		this->bindWidgetRoleEdit(roleCurr);
	});
	// open user edit widget when double clicked
	QObject::connect(m_roleTable, &QUaRoleTable::roleDoubleClicked, this,
	[this](QUaRole* role) {
		Q_UNUSED(role);
		this->showDock(QUaAcDockWidgets<T>::m_strRoleEdit);
	});
	QObject::connect(m_roleTable, &QUaRoleTable::roleEditClicked, this,
	[this](QUaRole* role) {
		Q_UNUSED(role);
		this->showDock(QUaAcDockWidgets<T>::m_strRoleEdit);
	});
}

template<class T>
inline void QUaAcDockWidgets<T>::setupPermsWidgets()
{
	Q_CHECK_PTR(m_permsTable);
	Q_CHECK_PTR(m_permsWidget);
	// disable until some valid object selected
	m_permsWidget->setEnabled(false);
	// get access control
	QUaAccessControl * ac = m_thiz->accessControl();
	// set ac to table
	QObject::connect(m_thiz, &T::loggedUserChanged, m_permsTable, &QUaPermissionsTable::on_loggedUserChanged);
	m_permsTable->setAccessControl(ac);
	// handle table events (perms selection)
	// change widgets
	QObject::connect(m_permsTable, &QUaPermissionsTable::permissionsSelectionChanged, this,
	[this](QUaPermissions * permsPrev, QUaPermissions * permsCurr)
	{
		Q_UNUSED(permsPrev);
		// early exit
		if (!permsCurr || m_thiz->isDeleting())
		{
			return;
		}
		// bind widget for current selection
		this->bindWidgetPermissionsEdit(permsCurr);
	});
	// open perms edit widget when double clicked
	QObject::connect(m_permsTable, &QUaPermissionsTable::permissionsDoubleClicked, this,
	[this](QUaPermissions* perms) {
		Q_UNUSED(perms);
		this->showDock(QUaAcDockWidgets<T>::m_strPermissionsEdit);
	});
	QObject::connect(m_permsTable, &QUaPermissionsTable::permissionsEditClicked, this,
	[this](QUaPermissions* perms) {
		Q_UNUSED(perms);
		this->showDock(QUaAcDockWidgets<T>::m_strPermissionsEdit);
	});
}

template<class T>
inline void QUaAcDockWidgets<T>::clearWidgetUserEdit()
{
	// disable old connections
	while (m_connsUserWidget.count() > 0)
	{
		QObject::disconnect(m_connsUserWidget.takeFirst());
	}
	m_userWidget->setUserName("");
	m_userWidget->setRole    (nullptr);
	m_userWidget->setPassword("");
	m_userWidget->setRepeat  ("");
	m_userWidget->setHash    ("");
	m_userWidget->setEnabled (false);
}

template<class T>
inline void QUaAcDockWidgets<T>::bindWidgetUserEdit(QUaUser * user)
{
	// disable old connections
	while (m_connsUserWidget.count() > 0)
	{
		QObject::disconnect(m_connsUserWidget.takeFirst());
	}
	// bind common
	m_connsUserWidget <<
	QObject::connect(user, &QObject::destroyed, this,
	[this]() {
		if (m_thiz->isDeleting())
		{
			return;
		}
		// clear widget
		this->clearWidgetUserEdit();
	});
	// name
	m_userWidget->setUserNameReadOnly(true);
	m_userWidget->setUserName(user->getName());
	// role
	auto role = user->role();
	m_userWidget->setRole(role);
	m_connsUserWidget <<
	QObject::connect(user, &QUaUser::roleChanged, this,
	[this](QUaRole * role) {
		if (m_thiz->isDeleting())
		{
			return;
		}
		m_userWidget->setRole(role);
	});
	m_connsUserWidget <<
	QObject::connect(role, &QObject::destroyed, user,
	[this, user]() {
		if (m_thiz->isDeleting())
		{
			return;
		}
		m_userWidget->setRole(user->role());
	});
	// hash
	m_userWidget->setHash(user->getHash().toHex());
	m_connsUserWidget <<
	QObject::connect(user, &QUaUser::hashChanged, this,
	[this](const QByteArray &hash) {
		if (m_thiz->isDeleting())
		{
			return;
		}
		m_userWidget->setHash(hash.toHex());
	});
	// clear password
	m_userWidget->setPassword("");
	m_userWidget->setRepeat("");
	// on click reset
	m_connsUserWidget <<
	QObject::connect(m_userWidget, &QUaUserWidgetEdit::resetClicked, user,
	[this, user]() {
		this->bindWidgetUserEdit(user);
	});
	// on click delete
	m_connsUserWidget <<
	QObject::connect(m_userWidget, &QUaUserWidgetEdit::deleteClicked, user,
	[this, user]() {
		// ask for confirmation
		auto res = QMessageBox::warning(
			m_thiz, 
			tr("Delete User Confirmation"), 
			tr("Are you sure you want to delete user %1?").arg(user->getName()), 
			QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No
		);
		if (res != QMessageBox::StandardButton::Yes)
		{
			return;
		}
		// delete (emit signal on list)
		user->remove();
	});
	// on click apply
	m_connsUserWidget <<
	QObject::connect(m_userWidget, &QUaUserWidgetEdit::applyClicked, user,
	[this, user]() {
		// udpate role
		user->setRole(m_userWidget->role());
		// update password (if non empty)
		QString strNewPass = m_userWidget->password().trimmed();
		QString strRepeat  = m_userWidget->repeat().trimmed();
		if (strNewPass.isEmpty())
		{
			return;
		}
		// check pass and repeat
		if (strNewPass.compare(strRepeat, Qt::CaseSensitive) != 0)
		{
			QMessageBox::critical(m_thiz, tr("Edit User Error"), tr("Passwords do not match."), QMessageBox::StandardButton::Ok);
			return;
		}
		QString strError = user->setPassword(strNewPass);
		if (strError.contains("Error"))
		{
			QMessageBox::critical(m_thiz, tr("Edit User Error"), tr("Invalid new password. %1").arg(strError), QMessageBox::StandardButton::Ok);
		}
	});
	// set permissions
	this->updateWidgetUserEditPermissions(user);
}

template<class T>
inline void QUaAcDockWidgets<T>::updateWidgetUserEditPermissions(QUaUser * user)
{
	auto ac = m_thiz->accessControl();
	m_userWidget->setEnabled(true);
	// if no user then clear
	if (!m_thiz->loggedUser())
	{
		this->clearWidgetUserEdit();
		return;
	}
	// permission to delete user or modify role come from user list permissions
	auto listPerms = m_thiz->loggedUser()->list()->permissionsObject();
	if (!listPerms)
	{
		// NOTE : cannot delete logged or root user
		bool canDelete = user == m_thiz->loggedUser() ? false : user == ac->rootUser() ? false : true;
		// no perms set, means all permissions
		m_userWidget->setDeleteVisible(canDelete);
		m_userWidget->setRoleReadOnly(false);
	}
	else
	{
		// NOTE : cannot delete logged or root user
		bool canDelete = user == m_thiz->loggedUser() ? false : user == ac->rootUser() ? false : listPerms->canUserWrite(m_thiz->loggedUser());
		m_userWidget->setDeleteVisible(canDelete);
		m_userWidget->setRoleReadOnly(!listPerms->canUserWrite(m_thiz->loggedUser()));
	}

	// permission to change password comes from individual user permissions
	auto dispUser = m_thiz->loggedUser()->list()->user(m_userWidget->userName());
	if (!dispUser)
	{
		this->clearWidgetUserEdit();
		return;
	}
	auto dispPerms = dispUser->permissionsObject();
	// no perms set, means all permissions
	if (!dispPerms)
	{
		m_userWidget->setPasswordVisible(true);
		return;
	}
	if (!dispPerms->canUserRead(m_thiz->loggedUser()))
	{
		this->clearWidgetUserEdit();
		return;
	}
	if (dispPerms->canUserWrite(m_thiz->loggedUser()))
	{
		m_userWidget->setPasswordVisible(true);
	}
	else
	{
		m_userWidget->setPasswordVisible(false);
	}
}

template<class T>
inline void QUaAcDockWidgets<T>::clearWidgetRoleEdit()
{
	// disable old connections
	while (m_connsRoleWidget.count() > 0)
	{
		QObject::disconnect(m_connsRoleWidget.takeFirst());
	}
	m_roleWidget->setRoleName("");
	m_roleWidget->setUsers(QStringList());
	m_roleWidget->setEnabled(false);
}

template<class T>
inline void QUaAcDockWidgets<T>::bindWidgetRoleEdit(QUaRole * role)
{
	// disable old connections
	while (m_connsRoleWidget.count() > 0)
	{
		QObject::disconnect(m_connsRoleWidget.takeFirst());
	}
	// bind common
	m_connsRoleWidget <<
	QObject::connect(role, &QObject::destroyed, this,
	[this]() {
		if (m_thiz->isDeleting())
		{
			return;
		}
		// clear widget
		this->clearWidgetRoleEdit();
	});
	// name
	m_roleWidget->setRoleNameReadOnly(true);
	m_roleWidget->setRoleName(role->getName());
	// users
	QStringList listUsers;
	for (auto user : role->users())
	{
		QString strUserName = user->getName();
		listUsers << strUserName;
		// suscribe used destroyed
		m_connsRoleWidget <<
		QObject::connect(user, &QObject::destroyed, this,
		[this, strUserName]() {
			if (m_thiz->isDeleting())
			{
				return;
			}
			m_roleWidget->removeUser(strUserName);
		});
	}
	m_roleWidget->setUsers(listUsers);
	// subscribe user added
	m_connsRoleWidget <<
	QObject::connect(role, &QUaRole::userAdded, this,
	[this](QUaUser * user) {
		if (m_thiz->isDeleting())
		{
			return;
		}
		QString strUserName = user->getName();
		m_roleWidget->addUser(strUserName);
		// suscribe used destroyed
		m_connsRoleWidget <<
		QObject::connect(user, &QObject::destroyed, this,
		[this, strUserName]() {
			if (m_thiz->isDeleting())
			{
				return;
			}
			m_roleWidget->removeUser(strUserName);
		});
	});
	// subscribe user removed
	m_connsRoleWidget <<
	QObject::connect(role, &QUaRole::userRemoved, this,
	[this](QUaUser * user) {
		if (m_thiz->isDeleting())
		{
			return;
		}
		m_roleWidget->removeUser(user->getName());
	});
	// on click delete
	m_connsRoleWidget <<
	QObject::connect(m_roleWidget, &QUaRoleWidgetEdit::deleteClicked, role,
	[this, role]() {
		// ask for confirmation
		auto res = QMessageBox::warning(
			m_thiz,
			tr("Delete Role Confirmation"),
			tr("Are you sure you want to delete role %1?").arg(role->getName()),
			QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No
		);
		if (res != QMessageBox::StandardButton::Yes)
		{
			return;
		}
		// delete (emit signal on list)
		role->remove();
	});

	// set permissions
	this->updateWidgetRoleEditPermissions();
}

template<class T>
inline void QUaAcDockWidgets<T>::updateWidgetRoleEditPermissions()
{
	m_roleWidget->setEnabled(true);
	// if no user then clear
	if (!m_thiz->loggedUser())
	{
		this->clearWidgetRoleEdit();
		return;
	}
	// get access control
	QUaAccessControl * ac = m_thiz->accessControl();
	// permission to delete role and see users come from role list permissions
	auto dispRole = ac->roles()->role(m_roleWidget->roleName());
	if (!dispRole)
	{
		this->clearWidgetRoleEdit();
		return;
	}
	auto listPerms = dispRole->list()->permissionsObject();
	if (!listPerms)
	{
		// no perms set, means all permissions
		m_roleWidget->setActionsVisible(true);
		m_roleWidget->setUserListVisible(true);
	}
	else
	{
		m_roleWidget->setActionsVisible(listPerms->canUserWrite(m_thiz->loggedUser()));
		m_roleWidget->setUserListVisible(listPerms->canUserRead(m_thiz->loggedUser()));
	}
	auto dispPerms = dispRole->permissionsObject();
	// no perms set, means all permissions (only read apply to role, nothing to modify)
	if (!dispPerms)
	{
		return;
	}
	if (!dispPerms->canUserRead(m_thiz->loggedUser()))
	{
		this->clearWidgetRoleEdit();
	}
}

template<class T>
inline void QUaAcDockWidgets<T>::clearWidgetPermissionsEdit()
{
	// disable old connections
	while (m_connsPermsWidget.count() > 0)
	{
		QObject::disconnect(m_connsPermsWidget.takeFirst());
	}
	m_permsWidget->setId("");
	m_permsWidget->setRoleAccessMap(QUaRoleAccessMap());
	m_permsWidget->setUserAccessMap(QUaUserAccessMap());
	m_permsWidget->setEnabled(false);
}

template<class T>
inline void QUaAcDockWidgets<T>::bindWidgetPermissionsEdit(QUaPermissions * perms)
{
	// get access control
	QUaAccessControl * ac = m_thiz->accessControl();
	// disable old connections
	while (m_connsPermsWidget.count() > 0)
	{
		QObject::disconnect(m_connsPermsWidget.takeFirst());
	}
	// bind common
	m_connsPermsWidget <<
	QObject::connect(perms, &QObject::destroyed, this,
	[this]() {
		if (m_thiz->isDeleting())
		{
			return;
		}
		// clear widget
		this->clearWidgetPermissionsEdit();
	});
	// name
	m_permsWidget->setIdReadOnly(true);
	m_permsWidget->setId(perms->getId());

	// roles (all)
	QUaRoleAccessMap mapRoles;
	auto roles = ac->roles()->roles();
	for (auto role : roles)
	{
		mapRoles[role->getName()] = {
			perms->canRoleRead(role),
			perms->canRoleWrite(role)
		};
	}
	m_permsWidget->setRoleAccessMap(mapRoles);

	// users (all)
	QUaUserAccessMap mapUsers;
	auto users = ac->users()->users();
	for (auto user : users)
	{
		mapUsers[user->getName()] = {
			perms->canUserReadDirectly (user),
			perms->canUserWriteDirectly(user),
			perms->canRoleRead (user->role()),
			perms->canRoleWrite(user->role())
		};
	}
	m_permsWidget->setUserAccessMap(mapUsers);

	// role updates
	auto updateRole = [this, perms](QUaRole * role) {
		// role table
		m_permsWidget->updateRoleAccess(
			role->getName(),
			{ perms->canRoleRead(role), perms->canRoleWrite(role) }
		);
		// user table
		auto users = role->users();
		for (auto user : users)
		{
			m_permsWidget->updateUserAccess(
				user->getName(), 
				{
				perms->canUserReadDirectly (user),
				perms->canUserWriteDirectly(user),
				perms->canRoleRead (user->role()),
				perms->canRoleWrite(user->role())
				}
			);
		}
	};
	m_connsPermsWidget << QObject::connect(perms, &QUaPermissions::canReadRoleAdded   , this, updateRole, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(perms, &QUaPermissions::canReadRoleRemoved , this, updateRole, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(perms, &QUaPermissions::canWriteRoleAdded  , this, updateRole, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(perms, &QUaPermissions::canWriteRoleRemoved, this, updateRole, Qt::QueuedConnection);

	// user updates
	auto updateUser = [this, perms](QUaUser * user) {
		m_permsWidget->updateUserAccess(
			user->getName(), 
			{
			perms->canUserReadDirectly (user),
			perms->canUserWriteDirectly(user),
			perms->canRoleRead (user->role()),
			perms->canRoleWrite(user->role())
			}
		);
	};
	m_connsPermsWidget << QObject::connect(perms, &QUaPermissions::canReadUserAdded   , this, updateUser, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(perms, &QUaPermissions::canReadUserRemoved , this, updateUser, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(perms, &QUaPermissions::canWriteUserAdded  , this, updateUser, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(perms, &QUaPermissions::canWriteUserRemoved, this, updateUser, Qt::QueuedConnection);

	// user changes role
	for (auto user : users)
	{
		m_connsPermsWidget << QObject::connect(user, &QUaUser::roleChanged, this, 
		[updateUser, user]() {
			updateUser(user);
		}, Qt::QueuedConnection);
	}

	// on user or role added/removed
	auto resetPermsWidget = [this, perms]() {
		this->bindWidgetPermissionsEdit(perms);
	};
	// NOTE : queued to wait until user/role has name or has actually been deleted
	m_connsPermsWidget << QObject::connect(ac->roles(), &QUaRoleList::roleAdded  , perms, resetPermsWidget, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(ac->roles(), &QUaRoleList::roleRemoved, perms, resetPermsWidget, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(ac->users(), &QUaUserList::userAdded  , perms, resetPermsWidget, Qt::QueuedConnection);
	m_connsPermsWidget << QObject::connect(ac->users(), &QUaUserList::userRemoved, perms, resetPermsWidget, Qt::QueuedConnection);

	// on click apply
	m_connsPermsWidget <<
	QObject::connect(m_permsWidget, &QUaPermissionsWidgetEdit::applyClicked, perms,
	[this, ac, perms]() {
		// update roles access
		auto roleMap = m_permsWidget->roleAccessMap();
		QUaRoleAccessIter i(roleMap);
		while (i.hasNext())
		{
			i.next();		
			auto role = ac->roles()->role(i.key());
			Q_CHECK_PTR(role);
			i.value().canRead  ? perms->addRoleCanRead (role) : perms->removeRoleCanRead (role);
			i.value().canWrite ? perms->addRoleCanWrite(role) : perms->removeRoleCanWrite(role);
		}
		// update users access
		auto userMap = m_permsWidget->userAccessMap();
		QUaUserAccessIter k(userMap);
		while (k.hasNext())
		{
			k.next();
			auto user = ac->users()->user(k.key());
			Q_CHECK_PTR(user);
			k.value().canUserRead  ? perms->addUserCanRead (user) : perms->removeUserCanRead (user);
			k.value().canUserWrite ? perms->addUserCanWrite(user) : perms->removeUserCanWrite(user);
		}
	});
	// on click reset
	m_connsPermsWidget <<
	QObject::connect(m_permsWidget, &QUaPermissionsWidgetEdit::resetClicked, perms,
	[this, perms]() {
		this->bindWidgetPermissionsEdit(perms);
	});
	// on click delete
	m_connsPermsWidget <<
	QObject::connect(m_permsWidget, &QUaPermissionsWidgetEdit::deleteClicked, perms,
	[this, perms]() {
		// ask for confirmation
		auto res = QMessageBox::warning(
			m_thiz,
			tr("Delete Permissions Confirmation"),
			tr("Are you sure you want to delete permissions object %1?").arg(perms->getId()),
			QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No
		);
		if (res != QMessageBox::StandardButton::Yes)
		{
			return;
		}
		// delete (emit signal on list)
		perms->remove();
	});

	// set permissions
	this->updateWidgetPermissionsEditPermissions();
}

template<class T>
inline void QUaAcDockWidgets<T>::updateWidgetPermissionsEditPermissions()
{
	m_permsWidget->setEnabled(true);
	// if no user then clear
	if (!m_thiz->loggedUser())
	{
		this->clearWidgetPermissionsEdit();
		return;
	}
	// get access control
	QUaAccessControl * ac = m_thiz->accessControl();
	// permission to delete permissions and see role/user access come from role list permissions
	auto dispPerms = ac->permissions()->permission(m_permsWidget->id());
	if (!dispPerms)
	{
		this->clearWidgetPermissionsEdit();
		return;
	}
	auto listPerms = dispPerms->list()->permissionsObject();
	if (!listPerms)
	{
		// no perms set, means all permissions
		m_permsWidget->setActionsVisible(true);
		m_permsWidget->setAccessVisible(true);
	}
	else
	{
		m_permsWidget->setActionsVisible(listPerms->canUserWrite(m_thiz->loggedUser()));
		m_permsWidget->setAccessVisible(listPerms->canUserRead(m_thiz->loggedUser()));
	}
	auto dispPermsPerms = dispPerms->permissionsObject();
	// no perms set, means all permissions (only read apply to perms, nothing else left to modify)
	if (!dispPermsPerms)
	{
		return;
	}
	if (!dispPermsPerms->canUserRead(m_thiz->loggedUser()))
	{
		this->clearWidgetPermissionsEdit();
	}
}

template<class T>
inline QUaAcDocking * QUaAcDockWidgets<T>::getDockManager() const
{
	return m_thiz->getDockManager();
}

template<class T>
inline void QUaAcDockWidgets<T>::showDock(const QString& strDockName)
{
	auto manager = this->getDockManager();
	auto perms = manager->dockPermissions(strDockName);
	if (perms && !perms->canUserRead(m_thiz->loggedUser()))
	{
		QMessageBox::critical(
			m_thiz,
			tr("Permissions Error"),
			tr("You are not allowed to perform this action.\n\nPlease contact an administrator."),
			QMessageBox::StandardButton::Ok
		);
		return;
	}
	manager->setIsDockVisible(strDockName, true);
	manager->setIsDockActive(strDockName, true);
}

#endif // QUAACDOCKWIDGETS_H
