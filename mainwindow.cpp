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
	ui->groupBox->setEnabled(false);

	connect( &mktAPI, SIGNAL(comError(QString)), this, SLOT(onCommError(QString)) );
	connect( &mktAPI, SIGNAL(loginRequest(QString*,QString*)), this, SLOT(onLoginRequest(QString*,QString*)) );

	connect( &mktAPI, SIGNAL(comStateChanged(ROS::Comm::CommState)),
			 this, SLOT(onStateChanged(ROS::Comm::CommState)) );
	connect( &mktAPI, SIGNAL(loginStateChanged(ROS::Comm::LoginState)),
			 this, SLOT(onLoginChanged(ROS::Comm::LoginState)) );

	connect( &mktAPI, SIGNAL(comReceive(ROS::QSentence&)), this, SLOT(onReceive(ROS::QSentence&)) );
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

void MainWindow::on_pbConnect_clicked()
{
	if( mktAPI.isClosing() )
	{
		ui->pbConnect->setText("Forzando cierre");
		mktAPI.closeCom(true);
	}
	else
	if( mktAPI.isLoged() )
	{
		ui->pbConnect->setText("Cerrando conexión");
		mktAPI.closeCom();
	}
	else
	if( mktAPI.isConnected() || mktAPI.isConnecting() )
	{
		ui->pbConnect->setText("Cerrando conexión");
		mktAPI.closeCom();
	}
	else
	if( mktAPI.connectTo(ui->leIP->text(), (unsigned short)ui->sbPort->value()) )
		ui->pbConnect->setText("Cancelar");
}


void MainWindow::onStateChanged(ROS::Comm::CommState s)
{
	ui->groupBox->setEnabled(false);
	switch( s )
	{
	case ROS::Comm::Unconnected:
		ui->pbConnect->setText( tr("Conectar") );
		ui->lwResponses->addItem( tr("Desconectado") );
		break;
	case ROS::Comm::HostLookup:
		ui->pbConnect->setText( tr("Cancelar") ) ;
		ui->lwResponses->addItem( tr("Resolviendo URL") );
		break;
	case ROS::Comm::Connecting:
		ui->pbConnect->setText( tr("Cancelar") );
		ui->lwResponses->addItem( tr("Conectando al servidor") );
		break;
	case ROS::Comm::Connected:
		ui->pbConnect->setText( tr("Desconectar") );
		ui->lwResponses->addItem( tr("Conectado") );
		break;
	case ROS::Comm::Closing:
		ui->pbConnect->setText( tr("Forzar desconexión") );
		ui->lwResponses->addItem( tr("Cerrando conexión") );
		break;
	}
}

void MainWindow::onLoginChanged(ROS::Comm::LoginState s)
{
	switch( s )
	{
	case ROS::Comm::NoLoged:
		ui->lwResponses->addItem( tr("No está identificado en el servidor") );
		ui->groupBox->setEnabled(false);
		break;
	case ROS::Comm::LoginRequested:
		ui->lwResponses->addItem( tr("Usuario y contraseña pedidos") );
		ui->groupBox->setEnabled(false);
		break;
	case ROS::Comm::UserPassSended:
		ui->lwResponses->addItem( tr("Petición de login en curso") );
		ui->groupBox->setEnabled(false);
		break;
	case ROS::Comm::LogedIn:
		ui->lwResponses->addItem( tr("Logado al servidor") );
		ui->pbConnect->setText("Desconectar");
		ui->groupBox->setEnabled(true);
		break;
	}
}

void MainWindow::onLoginRequest(QString *user, QString *pass)
{
	*user = ui->leUser->text();
	*pass = ui->lePass->text();
}

void MainWindow::onCommError(const QString &error)
{
	ui->lwResponses->addItem("Comm Error: "+error);
}

void MainWindow::onReceive(ROS::QSentence &s)
{
	ui->lwResponses->addItem(s.toString());
}

void MainWindow::on_pbEnviar_clicked()
{
	if( ui->leCommand->text().isEmpty() )
		return;
	ROS::QSentence s(ui->leCommand->text(), QString(),
					 ui->leAttrib->text().split(','));
	ui->lwResponses->addItem(QString("Sentencia enviada. Tag=%1").arg(mktAPI.sendSentence(s)));
}
