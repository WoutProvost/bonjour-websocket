#include "serversocket.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../common/messagetype.h"

ServerSocket::ServerSocket(ServiceRepository &serviceRepository, const QString &name, const QString &address, quint16 port, bool verbose) :
	QObject(),
	serviceRepository(serviceRepository),
	verbose(verbose),
	webSocketServer(name, QWebSocketServer::NonSecureMode, this)
{
	// Observe the repository
	serviceRepository.setObserver(this);

	// Register event handlers
	connect(&webSocketServer, &QWebSocketServer::closed, this, &ServerSocket::onClosed);
	connect(&webSocketServer, &QWebSocketServer::newConnection, this, &ServerSocket::onClientConnected);

	// Start listening for incoming connections
	if(webSocketServer.listen(QHostAddress(address), port)) {
		if(verbose) qDebug() << "Listening on address" << address << "and port" << port;
	}
}

ServerSocket::~ServerSocket()
{
	// Stop listening for incoming connections
	webSocketServer.close();

	// Remove all clients and deallocate memory
	qDeleteAll(clients.begin(), clients.end());
	clients.clear();
}

void ServerSocket::onClosed()
{
	if(verbose) qDebug() << "Closed connection";

	// Quit the application
	QCoreApplication::quit();
}

void ServerSocket::onClientConnected()
{
	if(verbose) qDebug() << "Client connected";

	// Get the new websocket connection
	QWebSocket *client = webSocketServer.nextPendingConnection();

	// Register event handlers
	connect(client, &QWebSocket::disconnected, this, &ServerSocket::onClientDisconnected);
	connect(client, &QWebSocket::textMessageReceived, this, &ServerSocket::onTextMessageReceived);

	// Add the connection to the list of connected clients
	clients.append(client);

	// Send a list of all services to the client
	notifyClientAllServices(client);
}

void ServerSocket::onClientDisconnected()
{
	if(verbose) qDebug() << "Client disconnected";

	// Remove the connection from the list of connected clients and deallocate memory
	QWebSocket *client = qobject_cast<QWebSocket *>(sender());
	if(client) {
		clients.removeAll(client);
		client->deleteLater();
	}
}

void ServerSocket::onTextMessageReceived(const QString &message)
{
	QWebSocket *client = qobject_cast<QWebSocket *>(sender());

	if(client) {
		QJsonDocument jsonDocument = QJsonDocument::fromJson(message.toUtf8());
		QJsonObject jsonMessage = jsonDocument.object();

		switch(jsonMessage["type"].toInt()) {
			case MessageType::REFRESH: {
				notifyClientAllServices(client);
				break;
			}
			default: {
				if(verbose) qDebug() << "Unsupported message type" << jsonMessage["type"].toInt();
				break;
			}
		}
	}
}

QJsonObject ServerSocket::createJsonService(const QMdnsEngine::Service &service)
{
	// Differentiate between service types of the same service
	QByteArray fullName = serviceRepository.getServiceFullName(service);

	QJsonObject jsonService;
	jsonService["name"] = QString(service.name());
	jsonService["hostname"] = QString(service.hostname());
	jsonService["port"] = service.port();
	jsonService["type"] = QString(service.type());
	jsonService["fullname"] = QString(fullName);

	QJsonObject jsonAttributes;
	QMap<QByteArray, QByteArray> attributes = service.attributes();
	for(auto it = attributes.begin(); it != attributes.end(); it++) {
		jsonAttributes[it.key()] = QString(it.value());
	}
	jsonService["attributes"] = jsonAttributes;

	QJsonArray jsonAddresses;
	for(const auto &address : serviceRepository.getAddresses()[fullName]) {
		jsonAddresses.append(address);
	}
	jsonService["addresses"] = jsonAddresses;

	return jsonService;
}

void ServerSocket::notifyClientAllServices(QWebSocket *client)
{
	QJsonArray jsonServices;
	QMap<QByteArray, QMdnsEngine::Service> &services = serviceRepository.getServices();
	for(auto it = services.begin(); it != services.end(); it++) {
		jsonServices.append(createJsonService(*it));
	}
	
	QJsonObject jsonMessage;
	jsonMessage["type"] = MessageType::ALL;
	jsonMessage["services"] = jsonServices;

	QJsonDocument jsonDocument(jsonMessage);
	client->sendTextMessage(jsonDocument.toJson());
}

void ServerSocket::onAddOrUpdateService(const QMdnsEngine::Service &service)
{
	QJsonObject jsonMessage;
	jsonMessage["type"] = MessageType::ADD_OR_UPDATE;
	jsonMessage["service"] = createJsonService(service);

	QJsonDocument jsonDocument(jsonMessage);
	QByteArray json = jsonDocument.toJson();

	for(const auto &client : clients) {
		client->sendTextMessage(json);
	}
}

void ServerSocket::onRemoveService(const QString &fullName)
{
	QJsonObject jsonMessage;
	jsonMessage["type"] = MessageType::REMOVE;
	jsonMessage["fullname"] = fullName;

	QJsonDocument jsonDocument(jsonMessage);
	QByteArray json = jsonDocument.toJson();

	for(const auto &client : clients) {
		client->sendTextMessage(json);
	}
}