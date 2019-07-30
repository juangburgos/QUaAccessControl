#ifndef QADDOCKLAYOUTBAR_H
#define QADDOCKLAYOUTBAR_H

#include <QWidget>
#include <QStandardItemModel>
#include <QUaAcCommonWidgets>

#include <QUaAcDocking>

class QUaAccessControl;
class QUaPermissions;

namespace Ui {
class QAdDockLayoutBar;
}

class QAdDockLayoutBar : public QWidget
{
    Q_OBJECT

public:
    explicit QAdDockLayoutBar(QWidget               * parent,
		                      QSortFilterProxyModel * permsFilter);
    ~QAdDockLayoutBar();

	void setLayouts(const QUaAcLayouts &mapLayouts);

signals:
	void setLayout           (const QString &strLayoutName);
	void saveCurrentLayout   ();
	void saveAsCurrentLayout ();
	void removeCurrentLayout ();
	void setLayoutPermissions(const QString &strLayoutName, QUaPermissions * permissions);

public slots:
	void on_layoutAdded             (const QString &strLayoutName);
	void on_layoutRemoved           (const QString &strLayoutName);
	void on_currentLayoutChanged    (const QString &strLayoutName);
	void on_layoutPermissionsChanged(const QString &strLayoutName, QUaPermissions * permissions);
	void on_layoutListPermissionsChanged(QUaPermissions * permissions);
	// TODO : N/A until stateChanged signal implemented
	// https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System/issues/43
	//void on_layoutUpdated    (const QString &strLayoutName);

	void on_loggedUserChanged(QUaUser * user);

private slots:
    void on_pushButtonSave_clicked();

    void on_pushButtonSaveAs_clicked();

    void on_pushButtonRemove_clicked();

    void on_pushButtonPermissions_clicked();

    void on_comboBoxLayout_currentIndexChanged(int index);

private:
    Ui::QAdDockLayoutBar *ui;

	QStandardItemModel    * m_modelPerms;
	QSortFilterProxyModel * m_proxyPerms;

	QStandardItemModel     m_modelLayouts;
	QUaAcLambdaFilterProxy m_proxyLayouts;

	QUaPermissions * m_layoutListPerms;

	QUaUser        * m_loggedUser;

	void updateLayoutListPermissions();
	void setLayoutActionsCanWrite(const bool &canWrite);
};

#endif // QADDOCKLAYOUTBAR_H
