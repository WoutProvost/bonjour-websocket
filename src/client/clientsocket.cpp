#include "clientsocket.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../common/messagetype.h"

ClientSocket::ClientSocket(QString url, int maxRetries, int retryInterval, int refreshInterval) :
	QObject(),
	url(url),
	maxRetries(maxRetries),
	retryInterval(retryInterval),
	refreshInterval(refreshInterval),
	connected(false),
	retries(0),
	webSocket(QString(), QWebSocketProtocol::VersionLatest, this),
	refreshTimer(this)
{
	qDebug() << "Searching for server on" << url;

	// Register event handlers
	connect(&webSocket, &QWebSocket::connected, this, &ClientSocket::onConnected);
	connect(&webSocket, &QWebSocket::disconnected, this, &ClientSocket::onDisconnected);
	connect(&webSocket, &QWebSocket::stateChanged, this, &ClientSocket::onStateChanged);
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
	qDebug() << "Connected";

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
	connected
		? qDebug() << "Disconnected"
		: qDebug();

	// Stop the refresh timer
	if(connected) {
		refreshTimer.stop();
	}

	// Retry connection after a few ms if the maximum amount of retries hasn't been reached
	if(maxRetries < 0 || retries < maxRetries) {
		connected
			? qDebug() << "Reconnecting ..."
			: qDebug() << "Timeout. Retrying ...";

		QTimer::singleShot(retryInterval, this, &ClientSocket::onReconnect);
		retries++;
	}
	// Quit the application
	else {
		maxRetries != 0
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

void ClientSocket::onStateChanged(QAbstractSocket::SocketState state)
{
	// qDebug() << "State changed to" << QVariant::fromValue(state).toString();
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
			removeService(jsonMessage["fullname"].toString().toUtf8());
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
	auto fullName = jsonService["fullname"].toString().toUtf8();

	services.contains(fullName)
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

	// Delete all addresses of this service first
	addresses[fullName].clear();

	QJsonArray jsonAddresses = jsonService["addresses"].toArray();
	for(auto it = jsonAddresses.begin(); it != jsonAddresses.end(); it++) {
		addresses[fullName].append(it->toString().toUtf8());
	}

	services[fullName] = service;

	printService(service);
}

void ClientSocket::removeService(const QByteArray &fullName)
{
	// Differentiate between service types of the same service
	// fullName

	services.contains(fullName)
		? qDebug() << "\e[31mREMOVED\e[0m" << fullName
		: qDebug();

	services.remove(fullName);
	addresses.remove(fullName);
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
	auto fullName = service.name() + "." + service.type();

	qDebug() << "-----" << fullName << "-----";

	qDebug() << "\e[34mINFO\e[0m" << "Name:" << service.name();
	qDebug() << "\e[34mINFO\e[0m" << "Hostname:" << service.hostname();
	qDebug() << "\e[34mINFO\e[0m" << "Port:" << service.port();
	qDebug() << "\e[34mINFO\e[0m" << "Type:" << service.type();
	qDebug() << "\e[34mINFO\e[0m" << "Fullname:" << fullName;

	auto attributes = service.attributes();
	qDebug() << "\e[34mINFO\e[0m" << "Attributes:" << (attributes.empty() ? "none" : "");
	for(auto it = attributes.begin(); it != attributes.end(); it++) {
		qDebug() << "\e[34mINFO\e[0m" << "\t" << it.key() << "->" << it.value();
	}

	auto addrs = addresses[fullName];
	qDebug() << "\e[34mINFO\e[0m" << "Addresses:" << (addrs.empty() ? "none" : "");
	for(auto it = addrs.begin(); it != addrs.end(); it++) {
		qDebug() << "\e[34mINFO\e[0m" << "\t" << *it;
	}
}