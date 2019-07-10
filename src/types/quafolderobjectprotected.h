#ifndef QUAFOLDEROBJECTPROTECTED_H
#define QUAFOLDEROBJECTPROTECTED_H

#include <QUaFolderObject>

class QUaPermissions;

class QUaFolderObjectProtected : public QUaFolderObject
{
    Q_OBJECT
public:
	Q_INVOKABLE explicit QUaFolderObjectProtected(QUaServer *server);

	// UA methods

	Q_INVOKABLE QString setPermissions(QList<QString> strPermissionsPath);

	Q_INVOKABLE QString clearPermissions();

	// C++ API

	bool              hasPermissionsObject() const;
	QUaPermissions  * permissionsObject() const;
	void              setPermissionsObject(QUaPermissions * permissions);

	// Reimplement these methods to define default user access for all instances of this type

	QUaAccessLevel userAccessLevel(const QString &strUserName) override;

	bool userExecutable(const QString &strUserName) override;

signals:
	// C++ API
	void permissionsObjectChanged(QUaPermissions * permissions);

public slots:
};

#endif // QUAFOLDEROBJECTPROTECTED_H