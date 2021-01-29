#ifndef MODEL_H
#define MODEL_H

#include <qmdnsengine/server.h>
#include <qmdnsengine/cache.h>
#include <qmdnsengine/browser.h>
#include <qmdnsengine/resolver.h>
#include <qmdnsengine/service.h>
#include "serversocket.h"

// Forward class declaration to prevent compiler from complaining
class ServerSocket;

class Model : public QObject
{
	Q_OBJECT

	private:
		QMdnsEngine::Server server;
		QMdnsEngine::Cache cache;
		QMdnsEngine::Browser browser;
		QMap<QPair<QByteArray, QByteArray>, QMdnsEngine::Resolver *> resolvers;

		ServerSocket *serverSocket;

		QMap<QPair<QByteArray, QByteArray>, QMdnsEngine::Service> services;
		QMap<QPair<QByteArray, QByteArray>, QList<QString>> addresses;

	public:
		Model(const QByteArray type = "_http._tcp.local.");
		~Model();

		void setServerSocket(ServerSocket *serverSocket);
		QMap<QPair<QByteArray, QByteArray>, QMdnsEngine::Service>* getServices();
		QMap<QPair<QByteArray, QByteArray>, QList<QString>>* getAddresses();

	private slots:
		void onServiceAdded(const QMdnsEngine::Service &service);
		void onServiceUpdated(const QMdnsEngine::Service &service);
		void onServiceRemoved(const QMdnsEngine::Service &service);
};

#endif