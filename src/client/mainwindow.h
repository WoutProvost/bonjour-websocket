#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "clientsocket.h"
#include "../common/servicerepository.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow, public Observer
{
	Q_OBJECT

	private:
		ServiceRepository &serviceRepository;
		ClientSocket &clientSocket;
		Ui::MainWindow *ui;

		void showExtras(const QString &fullName);

	public:
		MainWindow(ServiceRepository &serviceRepository, ClientSocket &clientSocket);
		~MainWindow();
		
		void onAddOrUpdateService(const QMdnsEngine::Service &service) override;
		void onRemoveService(const QString &fullName) override;

	private slots:
		void onSelectionChanged(const QString &fullName);
		void onRefreshTriggered();
};

#endif