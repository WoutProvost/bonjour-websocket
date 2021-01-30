#include <QCoreApplication>
#include <QCommandLineParser>
#include <qmdnsengine/mdns.h>
#include "model.h"
#include "serversocket.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	app.setApplicationName("Server");
	app.setApplicationVersion("1.0.0");
	app.setOrganizationDomain("example.com");
	app.setOrganizationName("Example");

	// Setup command line options
	QCommandLineParser parser;
	parser.setApplicationDescription("Bonjour/Zeroconf multicast DNS websocket server.");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addOption({{"t", "type"}, "The service type to browse for (default = any = _services._dns-sd._udp.local.).", "type", "_services._dns-sd._udp.local."});
	parser.addOption({{"n", "name"}, "The name to identify the server during the HTTP handshake (default = \"\").", "name", ""});
	parser.addOption({{"a", "address"}, "The address to listen to for incoming connections (default = any = 0.0.0.0).", "address", "0.0.0.0"});
	parser.addOption({{"p", "port"}, "The port to listen to for incoming connections (default = 1234).", "port", "1234"});
	parser.addOption({{"c", "no-cache"}, "Disable the use of a cache for DNS records."});
	parser.addOption({"verbose", "Displays debug information."});
	parser.process(app);

	// Parse command line options
	QString type = parser.value("t");
	QString name = parser.value("n");
	QString address = parser.value("a");
	int port = parser.value("p").toInt();
	bool noCache = parser.isSet("c");
	bool verbose = parser.isSet("verbose");

	// Create components
	Model model(type, noCache);
	ServerSocket serverSocket(&model, name, address, port, verbose);
	model.setServerSocket(&serverSocket);

	return app.exec();
}