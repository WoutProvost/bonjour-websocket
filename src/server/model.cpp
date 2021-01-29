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
	for(auto it = resolvers.begin(); it != resolvers.end(); it++) {
		qDeleteAll(it->begin(), it->end());
	}
	resolvers.clear();
}

void Model::setServerSocket(ServerSocket *serverSocket)
{
	this->serverSocket = serverSocket;
}

QMap<QByteArray, QMap<QByteArray, QMdnsEngine::Service>>* Model::getServices()
{
	return &services;
}

QMap<QByteArray, QMap<QByteArray, QList<QString>>>* Model::getAddresses()
{
	return &addresses;
}

void Model::onServiceAdded(const QMdnsEngine::Service &service)
{
	// Add the service to the list of services
	services[service.name()][service.type()] = service;

	// Remove any previous resolver from the list of resolvers and deallocate memory
	if(resolvers.contains(service.name()) && resolvers[service.name()].contains(service.type())) {
		resolvers[service.name()][service.type()]->deleteLater();
		resolvers[service.name()].remove(service.type());
	}

	// Resolve the service in order to connect to it, using a lambda expression
	auto resolver = new QMdnsEngine::Resolver(&server, service.hostname(), &cache, this);
	connect(resolver, &QMdnsEngine::Resolver::resolved, [this,service](const QHostAddress &address) {
		if(!addresses[service.name()][service.type()].contains(address.toString())) {
			// Add the address to the list of addresses of this service
			addresses[service.name()][service.type()].append(address.toString());
		
			// Notify clients
			serverSocket->notifyClientsAddOrUpdateService(service);
		}
	});

	// Add the resolver to the list of resolvers to prevent it going out of scope
	resolvers[service.name()][service.type()] = resolver;

	// Notify clients
	serverSocket->notifyClientsAddOrUpdateService(service);
}

void Model::onServiceUpdated(const QMdnsEngine::Service &service)
{
	// Replace the service in the list of services with new data
	services[service.name()][service.type()] = service;

	// Notify clients
	serverSocket->notifyClientsAddOrUpdateService(service);
}

void Model::onServiceRemoved(const QMdnsEngine::Service &service)
{
	// Delete the service from the list of services
	services[service.name()].remove(service.type());
	if(services[service.name()].empty()) {
		services.remove(service.name());
	}

	// Remove the resolver from the list of resolvers and deallocate memory
	resolvers[service.name()][service.type()]->deleteLater();
	resolvers[service.name()].remove(service.type());
	if(resolvers[service.name()].empty()) {
		resolvers.remove(service.name());
	}

	// Remove all the addresses of this service from the list of addresses
	addresses[service.name()].remove(service.type());
	if(addresses[service.name()].empty()) {
		addresses.remove(service.name());
	}

	// Notify clients
	serverSocket->notifyClientsRemoveService(service.name(), service.type());
}