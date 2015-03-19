#include "mainwindow.h"
#include "ui_mainwindow.h"

QConfigData gGlobalConfig;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent), ui(new Ui::MainWindow),
	mktAPI(this)
{
    ui->setupUi(this);

	// Rellenamos el diálogo con la configuración.
	gGlobalConfig.load();
	ui->leIP->setText(gGlobalConfig.getHost());
	ui->sbPort->setValue(gGlobalConfig.getPort());
	ui->leUser->setText(gGlobalConfig.getUserName());
	ui->lePass->setText(gGlobalConfig.getUserPass());

	connect( &mktAPI, SIGNAL(comConnected(bool)), SLOT(onConnected(bool)) );
	connect( &mktAPI, SIGNAL(addrFound()), this, SLOT(onURLResolved()) );
	connect( &mktAPI, SIGNAL(comError(QString)), this, SLOT(onCommError(QString)) );
	connect( &mktAPI, SIGNAL(loginRequest(QString*,QString*)), this, SLOT(onLoginRequest(QString*,QString*)) );
	connect( &mktAPI, SIGNAL(routerListening()), this, SLOT(onRouterListening()) );
	connect( &mktAPI, SIGNAL(comReceive(Mkt::QSentence&)), this, SLOT(onReceive(Mkt::QSentence&)) );
}

MainWindow::~MainWindow()
{
	mktAPI.closeCom();
	// Guardamos los datos del diálogo.
	gGlobalConfig.setHost(ui->leIP->text());
	gGlobalConfig.setPort(ui->sbPort->value());
	gGlobalConfig.setUserName(ui->leUser->text());
	gGlobalConfig.setUserPass(ui->lePass->text());
	gGlobalConfig.save();
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

void MainWindow::onReceive(Mkt::QSentence &s)
{
	ui->lwResponses->addItem(s.toString());
}

void MainWindow::onRouterListening()
{
	mktAPI.writeSentence("/interface/getall");
}
