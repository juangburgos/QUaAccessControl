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

/*---------------------------------------------------------------------------------------------
*/

QUaAcLambdaFilterProxy::QUaAcLambdaFilterProxy(QObject *parent/* = 0*/)
	: QSortFilterProxyModel(parent)
{

}

void QUaAcLambdaFilterProxy::resetFilter()
{
	this->invalidateFilter();
}

bool QUaAcLambdaFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
	return m_filterAcceptsRow ? m_filterAcceptsRow(sourceRow, sourceParent) : QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

bool QUaAcLambdaFilterProxy::lessThan(const QModelIndex & left, const QModelIndex & right) const
{
	return m_lessThan ? m_lessThan(left, right) : QSortFilterProxyModel::lessThan(left, right);
}

