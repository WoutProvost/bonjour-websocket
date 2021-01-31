#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScreen>

MainWindow::MainWindow(ServiceRepository &serviceRepository, ClientSocket &clientSocket) :
	QMainWindow(),
	serviceRepository(serviceRepository),
	clientSocket(clientSocket),
	ui(new Ui::MainWindow)
{
	// Observe the repository
	serviceRepository.setObserver(this);
	
	// Configure initial UI according to the generated UI header file
	ui->setupUi(this);

	// Set window title and center window on screen
	setWindowTitle("Client");
	move(QGuiApplication::primaryScreen()->geometry().center() - frameGeometry().center());

	// Register event handlers
	connect(ui->services, &QListWidget::currentTextChanged, this, &MainWindow::onSelectionChanged);
	connect(ui->refreshButton, &QToolButton::clicked, this, &MainWindow::onRefreshTriggered);
}

MainWindow::~MainWindow()
{
	// Delete UI
	delete ui;
}

void MainWindow::onSelectionChanged(const QString &fullName)
{
	// Display extras of the current selected service
	showExtras(fullName);
}

void MainWindow::onRefreshTriggered()
{
	// Remove all extras and services first
	ui->information->clearContents();
	ui->attributes->clearContents();
	ui->addresses->clear();
	ui->services->clear();
	
	// Manually ask the server for a refresh of the services
	clientSocket.refreshServices();
}

void MainWindow::showExtras(const QString &fullName)
{
	// Remove all extras first
	ui->information->clearContents();
	ui->attributes->clearContents();
	ui->addresses->clear();

	// Display extras
	if(!fullName.isEmpty()) {
		QMdnsEngine::Service &service = serviceRepository.getServices()[fullName.toUtf8()];

		ui->information->setItem(0, 0, new QTableWidgetItem("Name"));
		ui->information->setItem(0, 1, new QTableWidgetItem(QString(service.name())));
		ui->information->setItem(1, 0, new QTableWidgetItem("Hostname"));
		ui->information->setItem(1, 1, new QTableWidgetItem(QString(service.hostname())));
		ui->information->setItem(2, 0, new QTableWidgetItem("Port"));
		ui->information->setItem(2, 1, new QTableWidgetItem(QString::number(service.port())));
		ui->information->setItem(3, 0, new QTableWidgetItem("Type"));
		ui->information->setItem(3, 1, new QTableWidgetItem(QString(service.type())));
		ui->information->setItem(4, 0, new QTableWidgetItem("Fullname"));
		ui->information->setItem(4, 1, new QTableWidgetItem(fullName));
		ui->information->resizeColumnToContents(0);

		int row = 0;
		QMap<QByteArray, QByteArray> attributes = service.attributes();
		ui->attributes->setRowCount(attributes.size());
		for(auto it = attributes.begin(); it != attributes.end(); it++, row++) {
			ui->attributes->setItem(row, 0, new QTableWidgetItem(QString(it.key())));
			ui->attributes->setItem(row, 1, new QTableWidgetItem(QString(it.value())));
		}
		ui->information->resizeColumnToContents(0);

		for(const auto &address : serviceRepository.getAddresses()[fullName.toUtf8()]) {
			ui->addresses->addItem(address);
		}
	}
}

void MainWindow::onAddOrUpdateService(const QMdnsEngine::Service &service)
{
	// Differentiate between service types of the same service
	QByteArray fullName = serviceRepository.getServiceFullName(service);

	// Add the service to the list of services
	if(ui->services->findItems(fullName, Qt::MatchExactly).empty()) {
		ui->services->addItem(fullName);

		// Select the first item when the list was empty
		if(ui->services->count() == 1) {
			ui->services->setCurrentRow(0);
		}
	}
	// Update the service extras when the updated service is the current selected service
	else if(ui->services->currentItem()->text() == fullName) {
		showExtras(fullName);
	}
}

void MainWindow::onRemoveService(const QString &fullName)
{
	// Remove the service from the list of services
	delete ui->services->findItems(fullName, Qt::MatchExactly).first();
}