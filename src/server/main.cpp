#include <QCoreApplication>
#include "model.h"
#include "serversocket.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	// Create components
	Model model;
	ServerSocket serverSocket(&model);
	model.setServerSocket(&serverSocket);

	return app.exec();
}