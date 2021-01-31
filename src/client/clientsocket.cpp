#include "clientsocket.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../common/messagetype.h"

ClientSocket::ClientSocket(ServiceRepository &serviceRepository, const QString &url, int maxRetries, int retryInterval, int refreshInterval, bool verbose) :
	QObject(),
	serviceRepository(serviceRepository),
	url(url),
	maxRetries(maxRetries),
	retryInterval(retryInterval),
	refreshInterval(refreshInterval),
	verbose(verbose),
	connected(false),
	retries(0),
	webSocket(QString(), QWebSocketProtocol::VersionLatest, this),
	refreshTimer(this)
{
	if(verbose) qDebug() << "Searching for server on" << url;

	// Register event handlers
	connect(&webSocket, &QWebSocket::connected, this, &ClientSocket::onConnected);
	connect(&webSocket, &QWebSocket::disconnected, this, &ClientSocket::onDisconnected);
	connect(&webSocket, &QWebSocket::textMessageReceived, this, &ClientSocket::onTextMessageReceived);
	connect(&refreshTimer, &QTimer::timeout, this, &ClientSocket::refreshServices);

	// Open the websocket connection
	webSocket.open(url);
}

ClientSocket::~ClientSocket()
{
	// Close the websocket connection
	webSocket.close();
}

void ClientSocket::onConnected()
{
	if(verbose) qDebug() << "Connected";

	// Store that a connection was established and reset the amount of retries
	connected = true;
	retries = 0;

	// Start the refresh timer
	if(refreshInterval >= 0) {
		refreshTimer.start(refreshInterval);
	}
}

void ClientSocket::onDisconnected()
{
	if(verbose) connected
		? qDebug() << "Disconnected"
		: qDebug();

	// Stop the refresh timer
	if(connected && refreshInterval >= 0) {
		refreshTimer.stop();
	}

	// Retry connection after a few ms if the maximum amount of retries hasn't been reached
	if(maxRetries < 0 || retries < maxRetries) {
		if(verbose) connected
			? qDebug() << "Reconnecting ..."
			: qDebug() << "Timeout. Retrying ...";

		QTimer::singleShot(retryInterval, this, &ClientSocket::onReconnect);
		retries++;
	}
	// Quit the application
	else {
		if(verbose) maxRetries != 0
			? qDebug() << "Timeout. Maximum amount of retries reached. Quitting application"
			: qDebug();

		QCoreApplication::quit();
	}
}

void ClientSocket::onReconnect()
{
	// Store that a connection isn't established anymore
	if(connected) {
		connected = false;
	}

	// Reset and reopen the websocket connection
	webSocket.abort();
	webSocket.open(url);
}

void ClientSocket::onTextMessageReceived(const QString &message)
{
	QJsonDocument jsonDocument = QJsonDocument::fromJson(message.toUtf8());
	QJsonObject jsonMessage = jsonDocument.object();

	switch(jsonMessage["type"].toInt()) {
		case MessageType::ALL: {
			QMap<QByteArray, QMdnsEngine::Service> &services = serviceRepository.getServices();

			if(verbose) services.empty()
				? qDebug()
				: qDebug() << "\e[33mREFRESH\e[0m";
				
			// Remove all services and addresses first
			services.clear();
			serviceRepository.getAddresses().clear();

			for(const auto &jsonService : jsonMessage["services"].toArray()) {
				addOrUpdateService(jsonService.toObject());
			}
			break;
		}
		case MessageType::ADD_OR_UPDATE: {
			addOrUpdateService(jsonMessage["service"].toObject());
			break;
		}
		case MessageType::REMOVE: {
			removeService(jsonMessage["fullname"].toString().toUtf8());
			break;
		}
		default: {
			if(verbose) qDebug() << "Unsupported message type" << jsonMessage["type"].toInt();
			break;
		}
	}
}

void ClientSocket::addOrUpdateService(const QJsonObject &jsonService)
{
	// Differentiate between service types of the same service
	QByteArray fullName = jsonService["fullname"].toString().toUtf8();

	QMap<QByteArray, QMdnsEngine::Service> &services = serviceRepository.getServices();
	QMap<QByteArray, QList<QString>> &addresses = serviceRepository.getAddresses();

	if(verbose) services.contains(fullName)
		? qDebug() << "\e[33mUPDATED\e[0m" << fullName
		: qDebug() << "\e[32mADDED\e[0m" << fullName;

	QMdnsEngine::Service service;

	service.setName(jsonService["name"].toString().toUtf8());
	service.setHostname(jsonService["hostname"].toString().toUtf8());
	service.setPort(jsonService["port"].toInt());
	service.setType(jsonService["type"].toString().toUtf8());

	QJsonObject jsonAttributes = jsonService["attributes"].toObject();
	for(auto it = jsonAttributes.begin(); it != jsonAttributes.end(); it++) {
		service.addAttribute(it.key().toUtf8(), it.value().toString().toUtf8());
	}

	// Remove all addresses of this service first
	addresses[fullName].clear();

	for(const auto &jsonAddress : jsonService["addresses"].toArray()) {
		addresses[fullName].append(jsonAddress.toString().toUtf8());
	}

	services[fullName] = service;

	if(verbose) printService(service);
}

void ClientSocket::removeService(const QByteArray &fullName)
{
	// Differentiate between service types of the same service
	// fullName

	QMap<QByteArray, QMdnsEngine::Service> &services = serviceRepository.getServices();

	if(verbose) services.contains(fullName)
		? qDebug() << "\e[31mREMOVED\e[0m" << fullName
		: qDebug();

	services.remove(fullName);
	serviceRepository.getAddresses().remove(fullName);
}

void ClientSocket::refreshServices()
{
	// Restarts the refresh timer if the refresh was called manually
	if(!sender() && refreshInterval >= 0) {
		refreshTimer.start(refreshInterval);
	}

	QJsonObject jsonMessage;
	jsonMessage["type"] = MessageType::REFRESH;
	QJsonDocument jsonDocument(jsonMessage);
	webSocket.sendTextMessage(jsonDocument.toJson());
}

void ClientSocket::printService(const QMdnsEngine::Service &service)
{
	// Differentiate between service types of the same service
	QByteArray fullName = serviceRepository.getServiceFullName(service);

	qDebug() << "-----" << fullName << "-----";

	qDebug() << "\e[34mINFO\e[0m" << "Name:" << service.name();
	qDebug() << "\e[34mINFO\e[0m" << "Hostname:" << service.hostname();
	qDebug() << "\e[34mINFO\e[0m" << "Port:" << service.port();
	qDebug() << "\e[34mINFO\e[0m" << "Type:" << service.type();
	qDebug() << "\e[34mINFO\e[0m" << "Fullname:" << fullName;

	QMap<QByteArray, QByteArray> attributes = service.attributes();
	qDebug() << "\e[34mINFO\e[0m" << "Attributes:" << (attributes.empty() ? "none" : "");
	for(auto it = attributes.begin(); it != attributes.end(); it++) {
		qDebug() << "\e[34mINFO\e[0m" << "\t" << it.key() << "->" << it.value();
	}

	QList<QString> &addresses = serviceRepository.getAddresses()[fullName];
	qDebug() << "\e[34mINFO\e[0m" << "Addresses:" << (addresses.empty() ? "none" : "");
	for(const auto &address : addresses) {
		qDebug() << "\e[34mINFO\e[0m" << "\t" << address;
	}
}