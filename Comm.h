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

public:
	enum CommState
	{
		Unconnected,
		HostLookup,
		Connecting,
		Connected,
		Closing
	};

	enum LoginState
	{
		NoLoged,
		LoginRequested,
		UserPassSended,
		LogedIn
	};
	enum CommError
	{
		NoCommError,
		SocketError,
		LoginRefused,
		LogingSentenceEmpty,
		LogingSentenceNoRet,
		LogingSentenceRet32,
		LogingBadUsername,
		WordReceivedTooLong,
		WordToSendTooLong,
		ControlByteReceived
	};

private:
	QTcpSocket m_sock;
	QString m_addr;
	quint16 m_port;
	QString m_Username;
	QString m_Password;
	QByteArray incomingWord;
	QSentence incomingSentence;
	LoginState m_loginState;
	int incomingWordSize;
	int incomingWordCount;
	int incomingWordPos;
	int wordCount;
	char wordCountBuf[4];
	CommError lastCommError;

	void doLogin();
	void tryLogin();
	void sendUser();
	void setLoginState(LoginState s);
	void resetWord();
	void resetSentence();

	int receiveWordCount();
	int receiveWord();

	void sendWordCount(int wordCount);
	void sendWord(const QString &strWord);

	void setComError(CommError ce);

private slots:
	void onSocketError(QAbstractSocket::SocketError err);
	void receiveSentence();
	void onSocketStateChanges(QAbstractSocket::SocketState s);

signals:
	void comError(ROS::Comm::CommError ce, QAbstractSocket::SocketError se);
    void loginRequest(QString *user, QString *pass);
	void comReceive(ROS::QSentence &s);
	void comStateChanged(ROS::Comm::CommState s);
	void loginStateChanged(ROS::Comm::LoginState s);

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
	inline bool isClosing() const { return m_sock.state() == QAbstractSocket::ClosingState; }
	inline bool isConnecting() const { return m_sock.state() == QAbstractSocket::ConnectingState; }

	QString errorString();

public slots:
	QString sendSentence(const ROS::QSentence &sent, bool sendTag = true);
	QString sendSentence(const QString &cmd, bool sendTag = true, const QStringList &attrib = QStringList());

	void connectTo(const QString &addr, quint16 port);
	void closeCom(bool force = false);
};
}
#endif // APICOM_H
