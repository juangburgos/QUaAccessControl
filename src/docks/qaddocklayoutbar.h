#ifndef QADDOCKLAYOUTBAR_H
#define QADDOCKLAYOUTBAR_H

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

class QUaAccessControl;
class QUaPermissions;

namespace Ui {
class QAdDockLayoutBar;
}

class QAdDockLayoutBar : public QWidget
{
    Q_OBJECT

public:
    explicit QAdDockLayoutBar(QWidget *parent, QUaAccessControl *ac);
    ~QAdDockLayoutBar();

	void setLayoutNames(const QList<QString> &listLayoutNames);

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
	// TODO : N/A until stateChanged signal implemented
	// https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System/issues/43
	//void on_layoutUpdated    (const QString &strLayoutName);

private slots:
    void on_pushButtonSave_clicked();

    void on_pushButtonSaveAs_clicked();

    void on_pushButtonRemove_clicked();

    void on_pushButtonPermissions_clicked();

    void on_comboBoxLayout_currentIndexChanged(int index);

private:
    Ui::QAdDockLayoutBar *ui;

	QStandardItemModel    m_modelLayouts;
	QSortFilterProxyModel m_proxyLayouts;
};

#endif // QADDOCKLAYOUTBAR_H
