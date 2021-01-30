#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QWebSocket>
#include <QTimer>
#include <qmdnsengine/service.h>

class ClientSocket : public QObject
{
	Q_OBJECT

	private:
		QUrl url;
		int maxRetries;
		int retryInterval;
		int refreshInterval;
		bool verbose;
		bool connected;
		int retries;
		QWebSocket webSocket;
		QTimer refreshTimer;
		
		QMap<QByteArray, QMdnsEngine::Service> services;
		QMap<QByteArray, QList<QString>> addresses;

		void addOrUpdateService(const QJsonObject &jsonService);
		void removeService(const QByteArray &fullName);
		void refreshServices();
		void printService(const QMdnsEngine::Service &service);

	public:
		// URL: 'ws://' is the non-SSL version, 'wss://' is the SSL version
		ClientSocket(const QString &url, int maxRetries, int retryInterval, int refreshInterval, bool verbose);
		~ClientSocket();

	private slots:
		void onConnected();
		void onDisconnected();
		void onReconnect();
		void onTextMessageReceived(const QString &message);
};

#endif