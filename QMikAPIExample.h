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
	class QMikAPIExample;
}

class QConfigData
{
	QString configFName;
	QIniData iniData;

public:
	void defaults()
	{
		iniData["remoteHost"] = "192.168.1.1";
		iniData["remotePort"] = QString::number( 8728 );
		iniData["username"] = "admin";
		iniData["userpass"] = "";
		iniData["last-command"] = "/user/getall";
		iniData["last-query"] = "#|";
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

	void setLastCommand(const QString &cmd) { iniData["last-command"] = cmd;	}
	QString getLastCommand() const { return iniData["last-command"]; }

	void setLastQuery(const QString &cmd) { iniData["last-query"] = cmd;	}
	QString getLastQuery() const { return iniData["last-query"]; }

	void setLastAttributes(const QString &s) { iniData["last-attributes"] = s;	}
	QString getLastAttributes() const { return iniData["last-attributes"]; }

	void setLastAPIAttributes(const QString &s) { iniData["last-api-attributes"] = s;	}
	QString getLastAPIAttributes() const { return iniData["last-api-attributes"]; }

	void setLastTag(const QString &s) { iniData["last-tag"] = s;	}
	QString getLastTag() const { return iniData["last-tag"]; }

	void setAutoScrollToButton(bool b)	{ iniData["auto-scroll"] = b ? "true" : "false";	}
	bool getAutoScrollToButton()const	{ return iniData["auto-scroll"] == "true";	}
};

extern QConfigData gGlobalConfig;

class QMikAPIExample : public QMainWindow
{
    Q_OBJECT
	Ui::QMikAPIExample *ui;
	ROS::Comm mktAPI;

	void addLogText(const QString &txt);
public:
	explicit QMikAPIExample(QWidget *parent = 0);
	~QMikAPIExample();

private slots:
	void on_pbConnect_clicked();
	void onComError(ROS::Comm::CommError, QAbstractSocket::SocketError);
	void onReceive(ROS::QSentence &s);
	void onStateChanged(ROS::Comm::CommState s);
	void onLoginChanged(ROS::Comm::LoginState s);
	void on_pbEnviar_clicked();
};

#endif // MAINWINDOW_H
