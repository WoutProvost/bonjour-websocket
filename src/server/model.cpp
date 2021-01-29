#include "model.h"
#include <qmdnsengine/resolver.h>

Model::Model(const QByteArray type) :
	QObject(),
	server(this),
	cache(this),
	browser(&server, type, &cache, this)
{
	// Register event handlers
	connect(&browser, &QMdnsEngine::Browser::serviceAdded, this, &Model::onServiceAdded);
	connect(&browser, &QMdnsEngine::Browser::serviceUpdated, this, &Model::onServiceUpdated);
	connect(&browser, &QMdnsEngine::Browser::serviceRemoved, this, &Model::onServiceRemoved);
}

Model::~Model()
{
	// Delete all resolvers and deallocate memory
	qDeleteAll(resolvers.begin(), resolvers.end());
	resolvers.clear();
}

void Model::setServerSocket(ServerSocket *serverSocket)
{
	this->serverSocket = serverSocket;
}

QMap<QByteArray, QMdnsEngine::Service>* Model::getServices()
{
	return &services;
}

QMap<QByteArray, QList<QString>>* Model::getAddresses()
{
	return &addresses;
}

QByteArray Model::getServiceFullName(const QMdnsEngine::Service &service)
{
	return service.name() + "." + service.type();
}

void Model::onServiceAdded(const QMdnsEngine::Service &service)
{
	// Differentiate between service types of the same service
	auto fullName = getServiceFullName(service);

	// Add the service to the list of services
	services[fullName] = service;

	// Remove any previous resolver from the list of resolvers and deallocate memory
	if(resolvers.contains(fullName)) {
		resolvers[fullName]->deleteLater();
		resolvers.remove(fullName);
	}

	// Resolve the service in order to connect to it, using a lambda expression
	auto resolver = new QMdnsEngine::Resolver(&server, service.hostname(), &cache, this);
	connect(resolver, &QMdnsEngine::Resolver::resolved, [this,fullName,service](const QHostAddress &address) {
		if(!addresses[fullName].contains(address.toString())) {
			// Add the address to the list of addresses of this service
			addresses[fullName].append(address.toString());
		
			// Notify clients
			serverSocket->notifyClientsAddOrUpdateService(service);
		}
	});

	// Add the resolver to the list of resolvers to prevent it going out of scope
	resolvers[fullName] = resolver;

	// Notify clients
	serverSocket->notifyClientsAddOrUpdateService(service);
}

void Model::onServiceUpdated(const QMdnsEngine::Service &service)
{
	// Differentiate between service types of the same service
	auto fullName = getServiceFullName(service);

	// Replace the service in the list of services with new data
	services[fullName] = service;

	// Notify clients
	serverSocket->notifyClientsAddOrUpdateService(service);
}

void Model::onServiceRemoved(const QMdnsEngine::Service &service)
{
	// Differentiate between service types of the same service
	auto fullName = getServiceFullName(service);

	// Delete the service from the list of services
	services.remove(fullName);

	// Remove the resolver from the list of resolvers and deallocate memory
	if(resolvers.contains(fullName)) {
		resolvers[fullName]->deleteLater();
		resolvers.remove(fullName);
	}

	// Remove all the addresses of this service from the list of addresses
	addresses.remove(fullName);

	// Notify clients
	serverSocket->notifyClientsRemoveService(fullName);
}