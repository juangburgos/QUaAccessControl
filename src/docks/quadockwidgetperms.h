#ifndef QUADOCKWIDGETPERMS_H
#define QUADOCKWIDGETPERMS_H

#include <QWidget>
#include <QSortFilterProxyModel>
#include <QCompleter>

namespace Ui {
class QUaDockWidgetPerms;
}

class QUaPermissions;

class QUaDockWidgetPerms : public QWidget
{
    Q_OBJECT

public:
    explicit QUaDockWidgetPerms(QWidget *parent = nullptr);
    ~QUaDockWidgetPerms();

	void setComboModel(QSortFilterProxyModel * proxy);
	QSortFilterProxyModel * comboModel() const;

	QUaPermissions * permissions() const;
	void setPermissions(const QUaPermissions * permissions);

	static int PointerRole;

signals:
	void showPermsClicked();

private slots:
	void on_currentIndexChanged(int index);

private:
    Ui::QUaDockWidgetPerms *ui;
	bool m_deleting;
	QList<QMetaObject::Connection> m_connections;
	void clearWidgetPermissionsEdit();
	void bindWidgetPermissionsEdit(const QUaPermissions * perms);
};

#endif // QUADOCKWIDGETPERMS_H
