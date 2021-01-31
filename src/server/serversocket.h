#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <QWebSocketServer>
#include <QWebSocket>
#include "../common/servicerepository.h"

class ServerSocket : public QObject, public Observer
{
	Q_OBJECT

	private:
		ServiceRepository &serviceRepository;
		bool verbose;

		QWebSocketServer webSocketServer;
		QList<QWebSocket *> clients;

		QJsonObject createJsonService(const QMdnsEngine::Service &service);
		void notifyClientAllServices(QWebSocket *client);

	public:
		ServerSocket(ServiceRepository &serviceRepository, const QString &name, const QString &address, quint16 port, bool verbose);
		~ServerSocket();

		void onAddOrUpdateService(const QMdnsEngine::Service &service) override;
		void onRemoveService(const QString &fullName) override;

	private slots:
		void onClosed();
		void onClientConnected();
		void onClientDisconnected();
		void onTextMessageReceived(const QString &message);
};

#endif