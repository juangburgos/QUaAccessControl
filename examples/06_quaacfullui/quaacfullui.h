#ifndef QUAACFULLUI_H
#define QUAACFULLUI_H

#include <QMainWindow>
#include <QMenuBar>

#include <QUaServer>
#include <QUaAcDockWidgets>

namespace Ui {
class QUaAcFullUi;
}

class QUaAcFullUi : public QMainWindow
{
    Q_OBJECT

public:
    explicit QUaAcFullUi(QWidget *parent = nullptr);
    ~QUaAcFullUi();

	// NOTE : all public methods (including signals) are T requirements

	QUaAcDocking * getDockManager() const;

	QUaAccessControl * accessControl() const;

	bool isDeleting() const;

	QUaUser * loggedUser() const;
	void      setLoggedUser(QUaUser * user);

signals:
	void loggedUserChanged(QUaUser * user);

private slots:
	void on_loggedUserChanged(QUaUser * user);

	void on_newConfig();
	void on_openConfig();
	void on_saveConfig();
	bool on_closeConfig();

private:
    Ui::QUaAcFullUi *ui;

	// window members
	QUaServer        m_server     ;
	bool             m_deleting   ;
	QString          m_strSecret  ;
	QString          m_strTitle   ;
	QUaUser        * m_loggedUser ;
	QUaAcDocking   * m_dockManager;

	// NOTE : template arg must fullfill T requirements
	QUaAcDockWidgets<QUaAcFullUi> * m_acWidgets;

	void setupInfoModel      ();
	void setupNativeDocks    ();
	void setupMenuBar        ();

	void login ();
	void logout();
	void showCreateRootUserDialog (QUaAcCommonDialog &dialog);
	void showUserCredentialsDialog(QUaAcCommonDialog &dialog);

	// permissions model for combobox
	QStandardItemModel    m_modelPerms;
	QSortFilterProxyModel m_proxyPerms;
	void setupPermsModel();

	// XML import / export
	QByteArray xmlConfig();
	QString    setXmlConfig(const QByteArray &xmlConfig);

	QDomElement toDomElement(QDomDocument & domDoc) const;
	void        fromDomElement(QDomElement  & domElem, QString &strError);

	static QString m_strAppName;
	static QString m_strUntitiled;
	static QString m_strDefault;

	// to find children
	static QString m_strHelpMenu;
	static QString m_strTopDock;
};

#endif // QUAACFULLUI_H
