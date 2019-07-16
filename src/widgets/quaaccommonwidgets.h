#ifndef QUAACCOMMONWIDGETS_H
#define QUAACCOMMONWIDGETS_H

#include <QWidget>
#include <QComboBox>

class QUaAcReadOnlyComboBox : public QComboBox
{
	Q_OBJECT

public:
	explicit QUaAcReadOnlyComboBox(QWidget *parent = Q_NULLPTR);

	bool isReadOnly() const;
	void setReadOnly(const bool &bReadOnly);

protected:
	void mousePressEvent(QMouseEvent *e) override;
	void keyPressEvent(QKeyEvent   *e) override;
	void wheelEvent(QWheelEvent *e) override;

private:
	bool m_bReadOnly;
};

#endif // QUAACCOMMONWIDGETS_H