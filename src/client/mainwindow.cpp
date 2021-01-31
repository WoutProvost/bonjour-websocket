#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScreen>

MainWindow::MainWindow(ServiceRepository &serviceRepository) :
	QMainWindow(),
	serviceRepository(serviceRepository),
	ui(new Ui::MainWindow)
{
	// Observe the repository
	serviceRepository.setObserver(this);
	
	// Configure initial UI according to the generated UI header file
	ui->setupUi(this);

	// Set window title and center window on screen
	setWindowTitle("Client");
	move(QGuiApplication::primaryScreen()->geometry().center() - frameGeometry().center());
}

MainWindow::~MainWindow()
{
	// Delete UI
	delete ui;
}

void MainWindow::onAddOrUpdateService(const QMdnsEngine::Service &service)
{
}

void MainWindow::onRemoveService(const QString &fullName)
{
}