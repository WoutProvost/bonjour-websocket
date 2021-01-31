#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QWebSocket>
#include <QTimer>
#include "../common/servicerepository.h"

class ClientSocket : public QObject
{
	Q_OBJECT

	private:
		ServiceRepository &serviceRepository;
		QUrl url;
		int maxRetries;
		int retryInterval;
		int refreshInterval;
		bool verbose;

		bool connected;
		int retries;
		QWebSocket webSocket;
		QTimer refreshTimer;

		void addOrUpdateService(const QJsonObject &jsonService);
		void removeService(const QByteArray &fullName);
		void refreshServices();
		void printService(const QMdnsEngine::Service &service);

	public:
		// URL: 'ws://' is the non-SSL version, 'wss://' is the SSL version
		ClientSocket(ServiceRepository &serviceRepository, const QString &url, int maxRetries, int retryInterval, int refreshInterval, bool verbose);
		~ClientSocket();

	private slots:
		void onConnected();
		void onDisconnected();
		void onReconnect();
		void onTextMessageReceived(const QString &message);
};

#endif