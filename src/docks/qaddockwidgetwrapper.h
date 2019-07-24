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

	QWidget * widget() const;
	void      setWidget(QWidget * w);

private:
    Ui::QAdDockWidgetWrapper *ui;
};

#endif // QADDOCKWIDGETWRAPPER_H
