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

QMap<QPair<QByteArray, QByteArray>, QMdnsEngine::Service>* Model::getServices()
{
	return &services;
}

QMap<QPair<QByteArray, QByteArray>, QList<QString>>* Model::getAddresses()
{
	return &addresses;
}

void Model::onServiceAdded(const QMdnsEngine::Service &service)
{
	// Differentiate between service types of the same service
	auto pair = qMakePair(service.name(), service.type());

	// Add the service to the list of services
	services[pair] = service;

	// Remove any previous resolver from the list of resolvers and deallocate memory
	if(resolvers.contains(pair)) {
		resolvers[pair]->deleteLater();
		resolvers.remove(pair);
	}

	// Resolve the service in order to connect to it, using a lambda expression
	auto resolver = new QMdnsEngine::Resolver(&server, service.hostname(), &cache, this);
	connect(resolver, &QMdnsEngine::Resolver::resolved, [this,pair,service](const QHostAddress &address) {
		if(!addresses[pair].contains(address.toString())) {
			// Add the address to the list of addresses of this service
			addresses[pair].append(address.toString());
		
			// Notify clients
			serverSocket->notifyClientsAddOrUpdateService(service);
		}
	});

	// Add the resolver to the list of resolvers to prevent it going out of scope
	resolvers[pair] = resolver;

	// Notify clients
	serverSocket->notifyClientsAddOrUpdateService(service);
}

void Model::onServiceUpdated(const QMdnsEngine::Service &service)
{
	// Differentiate between service types of the same service
	auto pair = qMakePair(service.name(), service.type());

	// Replace the service in the list of services with new data
	services[pair] = service;

	// Notify clients
	serverSocket->notifyClientsAddOrUpdateService(service);
}

void Model::onServiceRemoved(const QMdnsEngine::Service &service)
{
	// Differentiate between service types of the same service
	auto pair = qMakePair(service.name(), service.type());

	// Delete the service from the list of services
	services.remove(pair);

	// Remove the resolver from the list of resolvers and deallocate memory
	if(resolvers.contains(pair)) {
		resolvers[pair]->deleteLater();
		resolvers.remove(pair);
	}

	// Remove all the addresses of this service from the list of addresses
	addresses.remove(pair);

	// Notify clients
	serverSocket->notifyClientsRemoveService(pair);
}