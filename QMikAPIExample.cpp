/*
	Copyright 2015 Rafael Dellà Bort. silderan (at) gmail (dot) com

	This file is part of QMikAPI.

	QMikAPI is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	QMikAPI is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	and GNU Lesser General Public License. along with QMikAPI.  If not,
	see <http://www.gnu.org/licenses/>.
 */

#include "QMikAPIExample.h"
#include "ui_QMikAPIExample.h"

QConfigData gGlobalConfig;

QMikAPIExample::QMikAPIExample(QWidget *parent) :
	QMainWindow(parent), ui(new Ui::QMikAPIExample),
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

	connect( &mktAPI, SIGNAL(comError(ROS::Comm::CommError,QAbstractSocket::SocketError)),
			 this, SLOT(onComError(ROS::Comm::CommError,QAbstractSocket::SocketError)) );

	connect( &mktAPI, SIGNAL(comStateChanged(ROS::Comm::CommState)),
			 this, SLOT(onStateChanged(ROS::Comm::CommState)) );
	connect( &mktAPI, SIGNAL(loginStateChanged(ROS::Comm::LoginState)),
			 this, SLOT(onLoginChanged(ROS::Comm::LoginState)) );

	connect( &mktAPI, SIGNAL(comReceive(ROS::QSentence&)), this, SLOT(onReceive(ROS::QSentence&)) );
}

QMikAPIExample::~QMikAPIExample()
{
	mktAPI.closeCom();
	// Guardamos los datos del diálogo.
	gGlobalConfig.setHost(ui->leIP->text());
	gGlobalConfig.setPort(ui->sbPort->value());
	gGlobalConfig.setUserName(ui->leUser->text());
	gGlobalConfig.setUserPass(ui->lePass->text());
	gGlobalConfig.save();
    delete ui;
	ui = 0;
}

void QMikAPIExample::addLogText(const QString &txt)
{
	if( ui )
	{
		ui->lwResponses->addItem(txt);
//		ui->lwResponses->scrollToBottom();
	}
}

void QMikAPIExample::on_pbConnect_clicked()
{
	if( mktAPI.isClosing() )
		mktAPI.closeCom(true);
	else
	if( mktAPI.isLoged() )
		mktAPI.closeCom();
	else
	if( mktAPI.isConnected() || mktAPI.isConnecting() )
		mktAPI.closeCom();
	else
	{
		gGlobalConfig.setUserName(ui->leUser->text());
		gGlobalConfig.setUserPass(ui->lePass->text());
		gGlobalConfig.setHost(ui->leIP->text());
		gGlobalConfig.setPort(ui->sbPort->value());
		mktAPI.setRemoteHost(gGlobalConfig.getHost(), gGlobalConfig.getPort());
		mktAPI.setUserNamePass(gGlobalConfig.getUserName(), gGlobalConfig.getUserPass());
		mktAPI.connectToROS();
	}
}


void QMikAPIExample::onStateChanged(ROS::Comm::CommState s)
{
	ui->groupBox->setEnabled(false);
	switch( s )
	{
	case ROS::Comm::Unconnected:
		ui->pbConnect->setText( tr("Conectar") );
		addLogText( tr("Desconectado") );
		break;
	case ROS::Comm::HostLookup:
		ui->pbConnect->setText( tr("Cancelar") ) ;
		addLogText( tr("Resolviendo URL") );
		break;
	case ROS::Comm::Connecting:
		ui->pbConnect->setText( tr("Cancelar") );
		addLogText( tr("Conectando al servidor") );
		break;
	case ROS::Comm::Connected:
		ui->pbConnect->setText( tr("Desconectar") );
		addLogText( tr("Conectado") );
		break;
	case ROS::Comm::Closing:
		ui->pbConnect->setText( tr("Forzar desconexión") );
		addLogText( tr("Cerrando conexión") );
		break;
	}
}

void QMikAPIExample::onLoginChanged(ROS::Comm::LoginState s)
{
	switch( s )
	{
	case ROS::Comm::NoLoged:
		addLogText( tr("No está identificado en el servidor") );
		ui->groupBox->setEnabled(false);
		break;
	case ROS::Comm::LoginRequested:
		addLogText( tr("Usuario y contraseña pedidos") );
		ui->groupBox->setEnabled(false);
		break;
	case ROS::Comm::UserPassSended:
		addLogText( tr("Petición de login en curso") );
		ui->groupBox->setEnabled(false);
		break;
	case ROS::Comm::LogedIn:
		addLogText( tr("Logado al servidor") );
		ui->pbConnect->setText("Desconectar");
		ui->groupBox->setEnabled(true);
		break;
	}
}

void QMikAPIExample::onComError(ROS::Comm::CommError, QAbstractSocket::SocketError)
{
	addLogText(mktAPI.errorString());
}

void QMikAPIExample::onReceive(ROS::QSentence &s)
{
	addLogText(s.toString());
}

void QMikAPIExample::on_pbEnviar_clicked()
{
	if( ui->leCommand->text().isEmpty() )
		return;
	ROS::QSentence s(ui->leCommand->text(), QString(),
					 ui->leAttrib->text().split(','),
					 ui->leAPI->text().split(','),
					 ui->leQueries->text().split(','));
	addLogText(QString("Sentencia enviada. Tag=%1").arg(mktAPI.sendSentence(s)));
}
