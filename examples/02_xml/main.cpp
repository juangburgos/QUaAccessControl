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

	server.objectsFolder()->addMethod("CallFunc", 
	[debouncedFunc](QString strName, int iAge) {
		debouncedFunc(strName, iAge);
	});

	server.start();

	return a.exec(); 
}
