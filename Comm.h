#ifndef APICOM_H
#define APICOM_H

#include <QObject>
#include <QtNetwork/qtcpsocket.h>
#include <QtNetwork/QHostAddress>

#include "QMD5.h"
#include "QSentences.h"

namespace ROS
{
class Comm : public QObject
{
    Q_OBJECT
    QTcpSocket m_sock;
    QString m_addr;
    quint16 m_port;
    QString m_Username;
    QString m_Password;
	QByteArray incomingWord;
	QSentence incomingSentence;
	bool sentenceCompleted;
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
	void writeLength(int wordLength);
    int readLength();
	void sendWord(const QString &strWord);
	int readWord();
	void readSentence();

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
	void comReceive(ROS::QSentence &s);
	void routerListening();

public:
	Comm(QObject *papi = NULL);
	~Comm();

	/**
	 * @brief isConnected
	 * This function just tells if socket is connectet to ROS, but
	 * doesn't takes care if logon was done.
	 * @return true/false if socket is in "ConnectetState" or not.
	 */
	inline bool isConnected() const { return m_sock.state() == QAbstractSocket::ConnectedState;	}
	/**
	 * @brief isLoged
	 * This function checks if a succefull login was done.
	 * Internally, checks isConnected. Si, will return false if is not connected.
	 * @return true/false if we are correctly loged into ROS.
	 */
	inline bool isLoged() const { return isConnected() && (m_loginState == LogedIn); }

public slots:
	QString sendSentence(const ROS::QSentence &sent, bool addTag = true);
	bool connectTo(const QString &addr, quint16 port);
	void closeCom() { if(isConnected()) m_sock.close(); }
};
}
#endif // APICOM_H
