#include <QCoreApplication>
#include <qmdnsengine/mdns.h>
#include "model.h"
#include "serversocket.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	// Create components
	Model model(QMdnsEngine::MdnsBrowseType);
	ServerSocket serverSocket(&model);
	model.setServerSocket(&serverSocket);

	return app.exec();
}