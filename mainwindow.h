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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Comm.h"
#include "QSentences.h"
#include "QIniFile.h"

namespace Ui
{
	class MainWindow;
}

class QConfigData
{
	QString configFName;
	QIniData iniData;

public:
	void defaults()
	{
		iniData["remoteHost"] = "192.168.1.1";
		iniData["remotePort"] = QString("%1").arg(8728);
		iniData["username"] = "admin";
		iniData["userpass"] = "";
	}

	QConfigData() : configFName("config.ini")
	{
		defaults();
	}
	~QConfigData()
	{
		save();
	}
	void load() { QIniFile::load(configFName, &iniData); }
	void save() const { QIniFile::save(configFName, iniData); }

	void setHost(const QString &host) { iniData["remoteHost"] = host; }
	QString getHost() const { return iniData["remoteHost"]; }

	void setPort(const quint16 &port) { iniData["remotePort"] = QString("%1").arg(port); }
	quint16 getPort() const { return iniData["remotePort"].toUShort(); }

	void setUserName(const QString &uname) { iniData["username"] = uname;	}
	QString getUserName() const { return iniData["username"]; }

	void setUserPass(const QString &upass) { iniData["userpass"] = upass;	}
	QString getUserPass() const { return iniData["userpass"]; }
};

extern QConfigData gGlobalConfig;

class MainWindow : public QMainWindow
{
    Q_OBJECT
	Ui::MainWindow *ui;
	ROS::Comm mktAPI;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
	void on_pbConnect_clicked();
	void onLoginRequest(QString *user, QString *pass);
	void onComError(ROS::Comm::CommError, QAbstractSocket::SocketError);
	void onReceive(ROS::QSentence &s);
	void onStateChanged(ROS::Comm::CommState s);
	void onLoginChanged(ROS::Comm::LoginState s);
	void on_pbEnviar_clicked();
};

#endif // MAINWINDOW_H
