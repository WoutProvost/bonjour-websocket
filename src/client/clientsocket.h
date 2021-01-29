#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QWebSocket>
#include <qmdnsengine/service.h>

class ClientSocket : public QObject
{
	Q_OBJECT

	private:
		QWebSocket webSocket;
		
		QMap<QPair<QByteArray, QByteArray>, QMdnsEngine::Service> services;
		QMap<QPair<QByteArray, QByteArray>, QList<QString>> addresses;

		void addOrUpdateService(const QJsonObject &jsonService);
		void removeService(const QJsonObject &jsonService);
		void refreshServices();
		void printService(const QMdnsEngine::Service &service);

	public:
		ClientSocket(QString url = "ws://localhost:1234"); // Non-SSL version
		~ClientSocket();

	private slots:
		void onConnected();
		void onDisconnected();
		void onStateChanged(QAbstractSocket::SocketState state);
		void onTextMessageReceived(const QString &message);
};

#endif