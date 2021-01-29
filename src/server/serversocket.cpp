#include "serversocket.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../common/messagetype.h"

ServerSocket::ServerSocket(Model *model, quint16 port) :
	QObject(),
	model(model),
	webSocketServer(QStringLiteral("Bonjour Server"), QWebSocketServer::NonSecureMode, this)
{
	// Register event handlers
	connect(&webSocketServer, &QWebSocketServer::closed, this, &ServerSocket::onClosed);
	connect(&webSocketServer, &QWebSocketServer::newConnection, this, &ServerSocket::onClientConnected);

	// Start listening for incoming connections
	if(webSocketServer.listen(QHostAddress::Any, port)) {
		qDebug() << "Listening on port" << port;
	}
}

ServerSocket::~ServerSocket()
{
	// Stop listening for incoming connections
	webSocketServer.close();

	// Delete all clients and deallocate memory
	qDeleteAll(clients.begin(), clients.end());
	clients.clear();
}

void ServerSocket::onClosed()
{
	qDebug() << "Closed connection";

	// Quit the application
	QCoreApplication::quit();
}

void ServerSocket::onClientConnected()
{
	qDebug() << "Client connected";

	// Get the new websocket connection
	auto client = webSocketServer.nextPendingConnection();

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
	qDebug() << "Client disconnected";

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
				qDebug() << "Unsupported message type" << jsonMessage["type"].toInt();
				break;
			}
		}
	}
}

QJsonObject ServerSocket::createJsonService(const QMdnsEngine::Service &service)
{
	QJsonObject jsonService;
	jsonService["name"] = QString(service.name());
	jsonService["hostname"] = QString(service.hostname());
	jsonService["port"] = service.port();
	jsonService["type"] = QString(service.type());

	QJsonObject jsonAttributes;
	auto attributes = service.attributes();
	for(auto it = attributes.begin(); it != attributes.end(); it++) {
		jsonAttributes[it.key()] = QString(it.value());
	}
	jsonService["attributes"] = jsonAttributes;

	QJsonArray jsonAddresses;
	auto addresses = &(*model->getAddresses())[service.name()][service.type()];
	for(auto it = addresses->begin(); it != addresses->end(); it++) {
		jsonAddresses.append(*it);
	}
	jsonService["addresses"] = jsonAddresses;

	return jsonService;
}

void ServerSocket::notifyClientAllServices(QWebSocket *client)
{
	auto services = model->getServices();

	QJsonArray jsonServices;
	for(auto it = services->begin(); it != services->end(); it++) {
		for(auto it2 = it->begin(); it2 != it->end(); it2++) {
			jsonServices.append(createJsonService(*it2));
		}
	}
	
	QJsonObject jsonMessage;
	jsonMessage["type"] = MessageType::ALL;
	jsonMessage["services"] = jsonServices;

	QJsonDocument jsonDocument(jsonMessage);
	client->sendTextMessage(jsonDocument.toJson());
}

void ServerSocket::notifyClientsAddOrUpdateService(const QMdnsEngine::Service &service)
{
	QJsonObject jsonMessage;
	jsonMessage["type"] = MessageType::ADD_OR_UPDATE;
	jsonMessage["service"] = createJsonService(service);

	QJsonDocument jsonDocument(jsonMessage);
	QByteArray json = jsonDocument.toJson();

	for(auto it = clients.begin(); it != clients.end(); it++) {
		(*it)->sendTextMessage(json);	
	}
}

void ServerSocket::notifyClientsRemoveService(const QString &name, const QString &type)
{
	QJsonObject jsonService;
	jsonService["name"] = name;
	jsonService["type"] = type;

	QJsonObject jsonMessage;
	jsonMessage["type"] = MessageType::REMOVE;
	jsonMessage["service"] = jsonService;

	QJsonDocument jsonDocument(jsonMessage);
	QByteArray json = jsonDocument.toJson();

	for(auto it = clients.begin(); it != clients.end(); it++) {
		(*it)->sendTextMessage(json);
	}
}