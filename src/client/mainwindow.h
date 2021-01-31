#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../common/servicerepository.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow, public Observer
{
	Q_OBJECT

	private:
		ServiceRepository &serviceRepository;
		Ui::MainWindow *ui;

	public:
		MainWindow(ServiceRepository &serviceRepository);
		~MainWindow();
		
		void onAddOrUpdateService(const QMdnsEngine::Service &service) override;
		void onRemoveService(const QString &fullName) override;
};

#endif