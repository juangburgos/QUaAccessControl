#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>
#include <FunctionUtils>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	// test debounce

	auto debouncedFunc = FunctionUtils::Debounce([](QString strName, int iAge) {
		qDebug() << strName << iAge;
	}, 2000);

	debouncedFunc("Juan", 31);
	debouncedFunc("Juan", 32);
	debouncedFunc("Juan", 33);

	server.objectsFolder()->addMethod("CallDebouncedFunc", 
	[debouncedFunc](QString strName, int iAge) {
		debouncedFunc(strName, iAge);
	});

	// test throttle

	auto throttledFunc = FunctionUtils::Throttle([](int iNum) {
		qDebug() << "throttle" << iNum;
	}, 5000);

	/*
	QTimer timer;
	QObject::connect(&timer, &QTimer::timeout, 
	[throttledFunc]() {
		static int iNum = 0;
		throttledFunc(iNum++);
	});
	timer.start(1000);
	*/

	server.objectsFolder()->addMethod("CallThrottledFunc",
	[throttledFunc](int iNum) {
		throttledFunc(iNum);
	});

	server.start();

	return a.exec(); 
}
