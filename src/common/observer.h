#ifndef OBSERVER_H
#define OBSERVER_H

#include <qmdnsengine/service.h>

class Observer
{
	public:
		virtual ~Observer() {}
		virtual void onAddOrUpdateService(const QMdnsEngine::Service &service) = 0;
		virtual void onRemoveService(const QString &fullName) = 0;
};

#endif