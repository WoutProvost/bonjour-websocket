#include <QApplication>
#include <QCommandLineParser>
#include "../common/servicerepository.h"
#include "clientsocket.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName("Client");
	app.setApplicationVersion("1.0.0");
	app.setOrganizationDomain("example.com");
	app.setOrganizationName("Example");

	// Setup command line options
	QCommandLineParser parser;
	parser.setApplicationDescription("Bonjour/Zeroconf multicast DNS websocket client.");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addOption({{"u", "url"}, "The URL to connect to (default = ws://localhost:1234).", "url", "ws://localhost:1234"});
	parser.addOption({{"m", "max-retries"}, "The maximum amount of reconnection attempts (default = unlimited = -1).", "max", "-1"});
	parser.addOption({{"r", "retry-interval"}, "The time to wait in ms before attempting a reconnect (default = 5000).", "interval", "5000"});
	parser.addOption({{"f", "refresh-interval"}, "The time to wait in ms before requesting a data refresh (default = unlimited = -1).", "interval", "-1"});
	parser.addOption({"verbose", "Displays debug information."});
	parser.process(app);

	// Parse command line options
	QString url = parser.value("u");
	int maxRetries = parser.value("m").toInt();
	int retryInterval = parser.value("r").toInt();
	int refreshInterval = parser.value("f").toInt();
	bool verbose = parser.isSet("verbose");

	// Create components
	ServiceRepository serviceRepository;
	ClientSocket clientSocket(serviceRepository, url, maxRetries, retryInterval, refreshInterval, verbose);

	// Create and show GUI
	MainWindow mainWindow(serviceRepository, clientSocket);
	mainWindow.show();

	return app.exec();
}