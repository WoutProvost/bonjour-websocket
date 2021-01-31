#ifndef SERVICEDISCOVERY_H
#define SERVICEDISCOVERY_H

#include <qmdnsengine/server.h>
#include <qmdnsengine/cache.h>
#include <qmdnsengine/browser.h>
#include <qmdnsengine/resolver.h>
#include "../common/servicerepository.h"

class ServiceDiscovery : public QObject
{
	Q_OBJECT

	private:
		ServiceRepository &serviceRepository;
		bool noCache;
		
		QMdnsEngine::Server server;
		QMdnsEngine::Cache cache;
		QMdnsEngine::Browser browser;
		QMap<QByteArray, QMdnsEngine::Resolver *> resolvers;

	public:
		ServiceDiscovery(ServiceRepository &serviceRepository, const QString &type, bool noCache);
		~ServiceDiscovery();

	private slots:
		void onServiceAdded(const QMdnsEngine::Service &service);
		void onServiceUpdated(const QMdnsEngine::Service &service);
		void onServiceRemoved(const QMdnsEngine::Service &service);
};

#endif