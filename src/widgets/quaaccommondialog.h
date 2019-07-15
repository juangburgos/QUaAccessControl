#ifndef QUAACCOMMONDIALOG_H
#define QUAACCOMMONDIALOG_H

#include <QDialog>

namespace Ui {
class QUaAcCommonDialog;
}

class QUaAcCommonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QUaAcCommonDialog(QWidget *parent = nullptr);
    ~QUaAcCommonDialog();

	QWidget * widget() const;
	void      setWidget(QWidget * w);

private:
    Ui::QUaAcCommonDialog *ui;
};

#endif // QUAACCOMMONDIALOG_H
