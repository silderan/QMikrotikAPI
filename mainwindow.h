#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "APICom.h"
#include "QSentences.h"

namespace Ui
{
	class MainWindow;
}

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
