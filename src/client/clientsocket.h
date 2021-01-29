#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QWebSocket>
#include <qmdnsengine/service.h>

class ClientSocket : public QObject
{
	Q_OBJECT

	private:
		QUrl url;
		int maxRetries;
		int retryInterval;
		bool connected;
		int retries;
		QWebSocket webSocket;
		
		QMap<QByteArray, QMdnsEngine::Service> services;
		QMap<QByteArray, QList<QString>> addresses;

		void addOrUpdateService(const QJsonObject &jsonService);
		void removeService(const QByteArray &fullName);
		void refreshServices();
		void printService(const QMdnsEngine::Service &service);

	public:
		ClientSocket(QString url = "ws://localhost:1234", int maxRetries = -1, int retryInterval = 5000); // 'ws' is non-SSL version, 'wss' is SSL version
		~ClientSocket();

	private slots:
		void onConnected();
		void onDisconnected();
		void onReconnect();
		void onStateChanged(QAbstractSocket::SocketState state);
		void onTextMessageReceived(const QString &message);
};

#endif