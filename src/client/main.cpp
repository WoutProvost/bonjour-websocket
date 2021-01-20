#include <QCoreApplication>
#include "clientsocket.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	// Create components
	ClientSocket clientSocket;

	return app.exec();
}