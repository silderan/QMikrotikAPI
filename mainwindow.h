#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "APICom.h"
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
	Mkt::APICom mktAPI;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
	void on_pushButton_clicked();
	void onConnected(bool succeful);
	void onLoginRequest(QString *user, QString *pass);
	void onURLResolved();
	void onCommError(const QString &error);
	void onReceive();
	void onRouterListening();
};



#endif // MAINWINDOW_H
