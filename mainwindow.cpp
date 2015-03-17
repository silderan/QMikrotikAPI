#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent), ui(new Ui::MainWindow),
	mktAPI(this)
{
    ui->setupUi(this);
	connect( &mktAPI, SIGNAL(comConnected(bool)), SLOT(onConnected(bool)) );
	connect( &mktAPI, SIGNAL(addrFound()), this, SLOT(onURLResolved()) );
	connect( &mktAPI, SIGNAL(comError(QString)), this, SLOT(onCommError(QString)) );
	connect( &mktAPI, SIGNAL(loginRequest(QString*,QString*)), this, SLOT(onLoginRequest(QString*,QString*)) );
	connect( &mktAPI, SIGNAL(routerListening()), this, SLOT(onRouterListening()) );
	connect( &mktAPI, SIGNAL(comReceive()), this, SLOT(onReceive()) );
}

MainWindow::~MainWindow()
{
	disconnect( &mktAPI );
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
	mktAPI.connectTo(ui->leIP->text(), (unsigned short)ui->sbPort->value());
}

void MainWindow::onConnected(bool succeful)
{
	if( succeful )
		ui->lwResponses->addItem("Conectado");
	else
		ui->lwResponses->addItem("Desconectado");
}

void MainWindow::onLoginRequest(QString *user, QString *pass)
{
	ui->lwResponses->addItem("Petición de Login.");
	*user = ui->leUser->text();
	*pass = ui->lePass->text();
}

void MainWindow::onURLResolved()
{
	ui->lwResponses->addItem("URL Resolved");
}

void MainWindow::onCommError(const QString &error)
{
	ui->lwResponses->addItem("Comm Error: "+error);
}

void MainWindow::onReceive()
{
	Mkt::QBlock block;
	// receive and print block from the API
	mktAPI.readBlock(block);
}

void MainWindow::onRouterListening()
{
	Mkt::QSentence sentence;

	// Fill and send sentence to the API
	sentence.append("/interface/getall");
	mktAPI.writeSentence(sentence);
}
