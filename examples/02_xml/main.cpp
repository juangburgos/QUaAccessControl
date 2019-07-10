#include <QCoreApplication>
#include <QDebug>

#include <QUaServer>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QUaServer server;

	// TODO

	server.start();

	return a.exec(); 
}
