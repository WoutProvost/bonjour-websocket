#ifndef SERVICEREPOSITORY_H
#define SERVICEREPOSITORY_H

#include "observer.h"

class ServiceRepository
{
	private:
		QMap<QByteArray, QMdnsEngine::Service> services;
		QMap<QByteArray, QList<QString>> addresses;
		Observer *observer;
	
	public:
		QMap<QByteArray, QMdnsEngine::Service>& getServices();
		QMap<QByteArray, QList<QString>>& getAddresses();
		QByteArray getServiceFullName(const QMdnsEngine::Service &service);
		
		void setObserver(Observer *observer);

		void notifyAddOrUpdateService(const QMdnsEngine::Service &service);
		void notifyRemoveService(const QString &fullName);
};

#endif