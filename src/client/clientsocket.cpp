#include "clientsocket.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../common/messagetype.h"

ClientSocket::ClientSocket(QString url) :
	QObject(),
	webSocket(QString(), QWebSocketProtocol::VersionLatest, this)
{
	qDebug() << "Searching for server on" << url;

	// Register event handlers
	connect(&webSocket, &QWebSocket::connected, this, &ClientSocket::onConnected);
	connect(&webSocket, &QWebSocket::disconnected, this, &ClientSocket::onDisconnected);
	connect(&webSocket, &QWebSocket::stateChanged, this, &ClientSocket::onStateChanged);

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
	qDebug() << "Connected";

	// Register event handlers
	connect(&webSocket, &QWebSocket::textMessageReceived, this, &ClientSocket::onTextMessageReceived);
}

void ClientSocket::onDisconnected()
{
	qDebug() << "Disconnected";

	// Quit the application
	QCoreApplication::quit();
}

void ClientSocket::onStateChanged(QAbstractSocket::SocketState state)
{
	qDebug() << "State changed to" << QVariant::fromValue(state).toString();
}

void ClientSocket::onTextMessageReceived(const QString &message)
{
	QJsonDocument jsonDocument = QJsonDocument::fromJson(message.toUtf8());
	QJsonObject jsonMessage = jsonDocument.object();

	switch(jsonMessage["type"].toInt()) {
		case MessageType::ALL: {
			services.empty()
				? qDebug()
				: qDebug() << "\e[33mREFRESH\e[0m";
				
			// Delete all services and addresses first
			services.clear();
			addresses.clear();

			QJsonArray jsonServices = jsonMessage["services"].toArray();
			for(auto it = jsonServices.begin(); it != jsonServices.end(); it++) {
				addOrUpdateService(it->toObject());
			}
			break;
		}
		case MessageType::ADD_OR_UPDATE: {
			addOrUpdateService(jsonMessage["service"].toObject());
			break;
		}
		case MessageType::REMOVE: {
			removeService(jsonMessage["service"].toObject());
			break;
		}
		default: {
			qDebug() << "Unsupported message type" << jsonMessage["type"].toInt();
			break;
		}
	}
}

void ClientSocket::addOrUpdateService(const QJsonObject &jsonService)
{
	// Differentiate between service types of the same service
	auto pair = qMakePair(jsonService["name"].toString().toUtf8(), jsonService["type"].toString().toUtf8());

	services.contains(pair)
		? qDebug() << "\e[33mUPDATED\e[0m" << jsonService["name"].toString() + " (" + jsonService["type"].toString() + ")"
		: qDebug() << "\e[32mADDED\e[0m" << jsonService["name"].toString() + " (" + jsonService["type"].toString() + ")";

	QMdnsEngine::Service service;

	service.setName(jsonService["name"].toString().toUtf8());
	service.setHostname(jsonService["hostname"].toString().toUtf8());
	service.setPort(jsonService["port"].toInt());
	service.setType(jsonService["type"].toString().toUtf8());

	QJsonObject jsonAttributes = jsonService["attributes"].toObject();
	for(auto it = jsonAttributes.begin(); it != jsonAttributes.end(); it++) {
		service.addAttribute(it.key().toUtf8(), it.value().toString().toUtf8());
	}

	// Delete all addresses of this service first
	addresses[pair].clear();

	QJsonArray jsonAddresses = jsonService["addresses"].toArray();
	for(auto it = jsonAddresses.begin(); it != jsonAddresses.end(); it++) {
		addresses[pair].append(it->toString().toUtf8());
	}

	services[pair] = service;

	printService(service);
}

void ClientSocket::removeService(const QJsonObject &jsonService)
{
	// Differentiate between service types of the same service
	auto pair = qMakePair(jsonService["name"].toString().toUtf8(), jsonService["type"].toString().toUtf8());

	services.contains(pair)
		? qDebug() << "\e[31mREMOVED\e[0m" << pair.first + " (" + pair.second + ")"
		: qDebug();

	services.remove(pair);
	addresses.remove(pair);
}

void ClientSocket::refreshServices()
{
	QJsonObject jsonMessage;
	jsonMessage["type"] = MessageType::REFRESH;
	QJsonDocument jsonDocument(jsonMessage);
	webSocket.sendTextMessage(jsonDocument.toJson());
}

void ClientSocket::printService(const QMdnsEngine::Service &service)
{
	// Differentiate between service types of the same service
	auto pair = qMakePair(service.name(), service.type());

	qDebug() << "-----" << service.name() + " (" + service.type() + ")" << "-----";

	qDebug() << "\e[34mINFO\e[0m" << "Name:" << service.name();
	qDebug() << "\e[34mINFO\e[0m" << "Hostname:" << service.hostname();
	qDebug() << "\e[34mINFO\e[0m" << "Port:" << service.port();
	qDebug() << "\e[34mINFO\e[0m" << "Type:" << service.type();

	auto attributes = service.attributes();
	qDebug() << "\e[34mINFO\e[0m" << "Attributes:" << (attributes.empty() ? "none" : "");
	for(auto it = attributes.begin(); it != attributes.end(); it++) {
		qDebug() << "\e[34mINFO\e[0m" << "\t" << it.key() << "->" << it.value();
	}

	auto addrs = addresses[pair];
	qDebug() << "\e[34mINFO\e[0m" << "Addresses:" << (addrs.empty() ? "none" : "");
	for(auto it = addrs.begin(); it != addrs.end(); it++) {
		qDebug() << "\e[34mINFO\e[0m" << "\t" << *it;
	}
}