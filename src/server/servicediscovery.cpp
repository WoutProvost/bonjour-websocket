#include "servicediscovery.h"
#include <qmdnsengine/resolver.h>

ServiceDiscovery::ServiceDiscovery(ServiceRepository &serviceRepository, const QString &type, bool noCache) :
	QObject(),
	serviceRepository(serviceRepository),
	noCache(noCache),
	server(this),
	cache(this),
	browser(&server, type.toUtf8(), noCache ? nullptr : &cache, this)
{
	// Register event handlers
	connect(&browser, &QMdnsEngine::Browser::serviceAdded, this, &ServiceDiscovery::onServiceAdded);
	connect(&browser, &QMdnsEngine::Browser::serviceUpdated, this, &ServiceDiscovery::onServiceUpdated);
	connect(&browser, &QMdnsEngine::Browser::serviceRemoved, this, &ServiceDiscovery::onServiceRemoved);
}

ServiceDiscovery::~ServiceDiscovery()
{
	// Remove all resolvers and deallocate memory
	qDeleteAll(resolvers.begin(), resolvers.end());
	resolvers.clear();
}

void ServiceDiscovery::onServiceAdded(const QMdnsEngine::Service &service)
{
	// Differentiate between service types of the same service
	QByteArray fullName = serviceRepository.getServiceFullName(service);

	// Remove any previous resolver from the list of resolvers and deallocate memory
	if(resolvers.contains(fullName)) {
		resolvers[fullName]->deleteLater();
		resolvers.remove(fullName);
	}

	// Resolve the service in order to connect to it, using a lambda expression
	QMdnsEngine::Resolver *resolver = new QMdnsEngine::Resolver(&server, service.hostname(), noCache ? nullptr : &cache, this);
	connect(resolver, &QMdnsEngine::Resolver::resolved, [this,fullName,service](const QHostAddress &address) {
		QMap<QByteArray, QList<QString>> &addresses = serviceRepository.getAddresses();

		// Prevent duplicate address entries for a service, if for some reason that address is resolved more than once
		if(!addresses[fullName].contains(address.toString())) {
			// Add the address to the list of addresses of this service
			addresses[fullName].append(address.toString());
		
			// Notify clients
			serviceRepository.notifyAddOrUpdateService(service);
		}
	});

	// Add the resolver to the list of resolvers to prevent it going out of scope
	resolvers[fullName] = resolver;

	// Add the service to the list of services
	serviceRepository.getServices()[fullName] = service;

	// Notify clients
	serviceRepository.notifyAddOrUpdateService(service);
}

void ServiceDiscovery::onServiceUpdated(const QMdnsEngine::Service &service)
{
	// Differentiate between service types of the same service
	QByteArray fullName = serviceRepository.getServiceFullName(service);

	// Replace the service in the list of services with new data
	serviceRepository.getServices()[fullName] = service;

	// Notify clients
	serviceRepository.notifyAddOrUpdateService(service);
}

void ServiceDiscovery::onServiceRemoved(const QMdnsEngine::Service &service)
{
	// Differentiate between service types of the same service
	QByteArray fullName = serviceRepository.getServiceFullName(service);

	// Remove the resolver from the list of resolvers and deallocate memory
	if(resolvers.contains(fullName)) {
		resolvers[fullName]->deleteLater();
		resolvers.remove(fullName);
	}

	// Remove the service from the list of services
	serviceRepository.getServices().remove(fullName);

	// Remove all the addresses of this service from the list of addresses
	serviceRepository.getAddresses().remove(fullName);

	// Notify clients
	serviceRepository.notifyRemoveService(fullName);
}