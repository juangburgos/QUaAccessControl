#ifndef QUAACCOMMONDIALOG_H
#define QUAACCOMMONDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QSharedPointer>

namespace Ui {
class QUaAcCommonDialog;
}

class QUaAcCommonDialog;

typedef QSharedPointer<QUaAcCommonDialog> QUaAcCommonDialogPtr;

class QUaAcCommonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QUaAcCommonDialog(QWidget *parent = nullptr);
    ~QUaAcCommonDialog();

	QWidget * widget() const;
	void      setWidget(QWidget * w);

	void clearButtons();
	void addButton(const QString &text, QDialogButtonBox::ButtonRole role);

	static QUaAcCommonDialogPtr CreateModal(QWidget *parent = nullptr);

signals:
	void dialogDestroyed();

private:
    Ui::QUaAcCommonDialog *ui;
};

#endif // QUAACCOMMONDIALOG_H
