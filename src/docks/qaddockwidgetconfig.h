#ifndef QADDOCKWIDGETCONFIG_H
#define QADDOCKWIDGETCONFIG_H

#include <QWidget>

namespace Ui {
class QAdDockWidgetConfig;
}

class QAdDockWidgetConfig : public QWidget
{
    Q_OBJECT

public:
    explicit QAdDockWidgetConfig(QWidget *parent = nullptr);
    ~QAdDockWidgetConfig();

	QWidget * configWidget() const;
	void      setConfigWidget(QWidget * w);

	QWidget * permissionsWidget() const;
	void      setPermissionsWidget(QWidget * w);

private:
    Ui::QAdDockWidgetConfig *ui;
};

#endif // QADDOCKWIDGETCONFIG_H
