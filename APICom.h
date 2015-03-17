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
	int incomingWordSize;
	QByteArray incomingWord;
	QSentence incomingSentence;
	enum LoginState
	{
		NoLoged,
		LoginRequested,
		UserPassSended,
		LogedIn
	} m_loginState;

	void doLogin();
	void tryLogin();
	void sendUser();

protected:
    void writeLength(int messageLength);
    int readLength();
    void writeWord(const QString &strWord);
	int readWord();

public:
    APICom(QObject *papi = NULL);
    ~APICom();

	bool connectTo(const QString &addr, quint16 port);
    void writeSentence(Mkt::QSentence &writeSentence);
	void readSentence();

    void readBlock(Mkt::QBlock &block);
	bool isConnected() const { return m_sock.state() == QAbstractSocket::ConnectedState;	}

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
	void comReceive();
	void routerListening();
};
}
#endif // APICOM_H
