#include "quaaccommonwidgets.h"

QUaAcReadOnlyComboBox::QUaAcReadOnlyComboBox(QWidget* parent/* = Q_NULLPTR*/)
	: QComboBox(parent)
{
	m_bReadOnly = false;
}

bool QUaAcReadOnlyComboBox::isReadOnly() const
{
	return m_bReadOnly;
}

void QUaAcReadOnlyComboBox::setReadOnly(const bool & bReadOnly)
{
	m_bReadOnly = bReadOnly;
}

void QUaAcReadOnlyComboBox::mousePressEvent(QMouseEvent * e)
{
	if (m_bReadOnly)
	{
		return;
	}
	QComboBox::mousePressEvent(e);
}

void QUaAcReadOnlyComboBox::keyPressEvent(QKeyEvent * e)
{
	if (m_bReadOnly)
	{
		return;
	}
	QComboBox::keyPressEvent(e);
}

void QUaAcReadOnlyComboBox::wheelEvent(QWheelEvent * e)
{
	if (m_bReadOnly)
	{
		return;
	}
	QComboBox::wheelEvent(e);
}