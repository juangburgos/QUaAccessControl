#ifndef QADDOCKWIDGETWRAPPER_H
#define QADDOCKWIDGETWRAPPER_H

#include <QWidget>

namespace Ui {
class QAdDockWidgetWrapper;
}

class QAdDockWidgetWrapper : public QWidget
{
    Q_OBJECT

public:
    explicit QAdDockWidgetWrapper(QWidget *parent = nullptr);
    ~QAdDockWidgetWrapper();

	bool isEditBarVisible() const;
	void setEditBarVisible(const bool &isVisible);

	bool isConfigButtonVisible() const;
	void seConfigButtonVisible(const bool &isVisible);

	QString title() const;
	void    setTitle(const QString &strTitle);

	QWidget * widget() const;
	void      setWidget(QWidget * w);

signals:
	void configClicked();
	void permissionsClicked();

private:
    Ui::QAdDockWidgetWrapper *ui;
};

#endif // QADDOCKWIDGETWRAPPER_H
