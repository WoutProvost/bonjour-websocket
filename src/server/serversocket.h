#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <QWebSocketServer>
#include <QWebSocket>
#include <qmdnsengine/service.h>
#include "model.h"

// Forward class declaration to prevent compiler from complaining
class Model;

class ServerSocket : public QObject
{
	Q_OBJECT

	private:
		QWebSocketServer webSocketServer;
		QList<QWebSocket *> clients;
		Model *model;

		QJsonObject createJsonService(const QMdnsEngine::Service &service);
		void notifyClientAllServices(QWebSocket *client);

	public:
		ServerSocket(Model *model, quint16 port = 1234);
		~ServerSocket();
		
		void notifyClientsAddOrUpdateService(const QMdnsEngine::Service &service);
		void notifyClientsRemoveService(const QString &name);

	private slots:
		void onClosed();
		void onClientConnected();
		void onClientDisconnected();
		void onTextMessageReceived(const QString &message);
};

#endif