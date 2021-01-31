#include "servicerepository.h"

QMap<QByteArray, QMdnsEngine::Service>& ServiceRepository::getServices()
{
	return services;
}

QMap<QByteArray, QList<QString>>& ServiceRepository::getAddresses()
{
	return addresses;
}

QByteArray ServiceRepository::getServiceFullName(const QMdnsEngine::Service &service)
{
	return service.name() + "." + service.type();
}

void ServiceRepository::setObserver(Observer *observer)
{
	this->observer = observer;
}

void ServiceRepository::notifyAddOrUpdateService(const QMdnsEngine::Service &service)
{
	observer->onAddOrUpdateService(service);
}

void ServiceRepository::notifyRemoveService(const QString &fullName)
{
	observer->onRemoveService(fullName);
}