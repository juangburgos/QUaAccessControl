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

#include <functional>
#include <QSortFilterProxyModel>

class QUaAcLambdaFilterProxy : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	QUaAcLambdaFilterProxy(QObject *parent = 0);

	void resetFilter();

	template<typename M>
	void setFilterAcceptsRow(const M &callback);

	template<typename M>
	void setLessThan(const M &callback);

protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
	std::function<bool(int, const QModelIndex &)> m_filterAcceptsRow;
	std::function<bool(const QModelIndex &, const QModelIndex &)> m_lessThan;
};

template<typename M>
inline void QUaAcLambdaFilterProxy::setFilterAcceptsRow(const M & callback)
{
	m_filterAcceptsRow = [callback](int sourceRow, const QModelIndex &sourceParent) {
		return callback(sourceRow, sourceParent);
	};
}

template<typename M>
inline void QUaAcLambdaFilterProxy::setLessThan(const M & callback)
{
	m_lessThan = [callback](const QModelIndex &left, const QModelIndex &right) {
		return callback(left, right);
	};
}

#endif // QUAACCOMMONWIDGETS_H
