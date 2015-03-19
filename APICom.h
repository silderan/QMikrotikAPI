#ifndef APICOM_H
#define APICOM_H

#include <QObject>
#include <QtNetwork/qtcpsocket.h>
#include <QtNetwork/QHostAddress>

#include "QMD5.h"
#include "QSentences.h"

namespace Mkt
{
class APICom : public QObject
{
    Q_OBJECT
    QTcpSocket m_sock;
    QString m_addr;
    quint16 m_port;
    QString m_Username;
    QString m_Password;
	QByteArray incomingWord;
	QSentence incomingSentence;
	enum LoginState
	{
		NoLoged,
		LoginRequested,
		UserPassSended,
		LogedIn
	} m_loginState;
	int incomingWordSize;

	void doLogin();
	void tryLogin();
	void sendUser();

protected:
    void writeLength(int messageLength);
    int readLength();
	void writeWord(const QString &strWord);
	int readWord();
	void readSentence();

public:
    APICom(QObject *papi = NULL);
    ~APICom();

//	void writeSentence(const Mkt::QSentence &writeSentence);
	void writeSentence(const QString &sentence);

	bool isConnected() const { return m_sock.state() == QAbstractSocket::ConnectedState;	}

public slots:
	bool connectTo(const QString &addr, quint16 port);
	void closeCom() { if(isConnected()) m_sock.close(); }

private slots:
	void onError(QAbstractSocket::SocketError);
	void onConnected();
	void onDisconnected();
	void onHostLookup();
	void onReadyRead();

signals:
    void comConnected(bool c);
	void comError(const QString &error);
    void addrFound();
    void loginRequest(QString *user, QString *pass);
	void comReceive(Mkt::QSentence &s);
	void routerListening();
};
}
#endif // APICOM_H
