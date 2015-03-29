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

#include "Comm.h"

using namespace ROS;

#include <QMessageBox>

Comm::Comm(QObject *papi)
 : QObject(papi), m_loginState(NoLoged), incomingWordSize(-1), lastCommError(NoCommError)
{
	connect( &m_sock, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(onSocketError(QAbstractSocket::SocketError)) );
	connect( &m_sock, SIGNAL(readyRead()), this, SLOT(receiveSentence()) );
	connect( &m_sock, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
			 this, SLOT(onSocketStateChanges(QAbstractSocket::SocketState)) );
}

Comm::~Comm()
{
	m_sock.close();
}

/**
 * @brief Comm::errorString
 * Returns a human-readable string descriving the last error
 * ocurred on communication.
 * This error can be a standard socket error (provided by QAbstractSocket)
 * or a custom one.
 * @return A human-readable string descriving the last error.
 */
QString Comm::errorString()
{
	switch( lastCommError )
	{
	case NoCommError:
		if( m_sock.error() != QAbstractSocket::UnknownSocketError )
			return m_sock.errorString();
		return tr("No error");
	case SocketError:
		return tr("A socket error.");
	case LoginRefused:
		return tr("Login refused from remote router");
	case LogingSentenceEmpty:
		return tr("Unknown remote login sentence format: didn't receive anything" );
	case LogingSentenceNoRet:
		return tr("Unknown remote login sentence format: Doesn't receive 'ret' namefield" );
	case LogingSentenceRet32:
		return tr("Unknown remote login sentence format: 'ret' field doesn't contains 32 characters" );
	case LogingBadUsername:
		return tr("Invalid Username or Password");
	case WordReceivedTooLong:
		return tr("Word length comming from socket is too long to be handled by this API");
	case WordToSendTooLong:
		return tr("Word length to be send to router is too long to be handled by this API");
	case ControlByteReceived:
		return tr("Control byte received from router. Cannot continue as it's a undocumented situation.");
	}
	return tr("Unknown error");
}

/**
 * @brief Comm::resetWord
 * Resets incomingWord and all variables used to receive the words from
 * router.
 * Call this function just after using new word received to allow
 * reveiving a new one.
 */
void Comm::resetWord()
{
	incomingWord.clear();
	incomingWordSize = -1;
	incomingWordPos = 0;
	incomingWordCount = -1;
	(*(int*)wordCountBuf) = 0;
}

/**
 * @brief Comm::resetSentence
 * Resets incomingSentence and all variables used to receive
 * the sentence from router.
 * Call this function just after using the sentence to allow
 * receiving a new one.
 */
void Comm::resetSentence()
{
	resetWord();
	incomingSentence.clear();
}

/**
 * @brief Comm::writeSentence
 * Sends a full sentence.
 * If there is no tag provided in sentence info, a unique one
 * is created to send to and returned.
 * @param sent Sentence class with the info to sent to Router.
 * @param sendTag (Optional, default==true) Tells function to use (or,
 * eventually create and use) a tag for sentence to sent.
 * @return tag used for sentence.
 */
QString Comm::sendSentence(const QSentence &sent, bool sendTag)
{
	static int ID = 0;
	QString word;

	sendWord(sent.command());
	if( !sent.getID().isEmpty() )
		sendWord(QString("=.id=%1").arg(sent.getID()));
	foreach( word, sent.attributes().toWords() )
		sendWord(word);
	foreach( word, sent.APIattributes().toWords() )
		sendWord(word);
	foreach( word, sent.queries().toWords() )
		sendWord(word);

	if( sendTag )
	{
		word = sent.tag();
		if( word.isEmpty() )
			word = QString("%1").arg(++ID);
		sendWord(QString(".tag=%1").arg(word));
	}
	else
		word.clear();
	sendWord("");
	return word;
}

/**
 * @brief Comm::sendSentence
 * Sends a new sentence.
 * This is a convenient function. Internally, just calls sendSentence(const QSentence&, bool)
 * @overload
 * @see sendSentence(const QSentence&, bool);
 */
QString Comm::sendSentence(const QString &cmd, bool sendTag, const QStringList &attrib)
{
	return sendSentence( QSentence(cmd, QString(), attrib), sendTag );
}

QString Comm::sendCancel(const QString &tag)
{
	return sendSentence( QSentence("/cancel", tag) );
}

QString Comm::sendSentence(const QString &cmd, const QString &tag, const QStringList &attrib)
{
	return sendSentence( QSentence(cmd, tag, attrib) );
}

/**
 * @brief Comm::sendWord
 * Sends a word to router.
 * Word lenght is calculated and is sended before word itself.
 * @param word
 */
void Comm::sendWord(const QString &word)
{
	sendWordCount( word.count() );
	m_sock.write( word.toLatin1() );
}

/**
 * @brief Comm::setComError
 * sets lastCommError variable.
 * If new error is not NoCommError and not the same as before,
 * comError will be emited.
 * @param ce The new error.
 */
void Comm::setComError(Comm::CommError ce)
{
	if( ce != NoCommError )
	{
		if( ce != lastCommError )
			emit comError(lastCommError = ce, QAbstractSocket::UnknownSocketError);
	}
	else
		lastCommError = ce;
}

/**
 * @brief Comm::receiveWordCount
 * Reads the word length from socket.
 * Lenngth readed is stored on incomingWordCount variable.
 * Only 4 bytes at most will be readed from socket. In case of more bytes
 * needed to read, connection will be closed.
 * It a control byte is received, connection will be closed too.
 * @return >0 if length was readed succefully.
 * <0 on socket error. Socket shall be closed as soon as possible.
 * == 0 no or incomplete data readed from socket. In this case, this function must be
 * called again when socket receives more data.
 */
int Comm::receiveWordCount()
{
	unsigned char c;
	int i = m_sock.read((char*)&c, 1);
	if( i <= 0 )
		return i;

	// The first byte received from socket has allways coded with the
	// amount of bytes used for word length.
	// So, if is the first byte, let's see how many bytes we need to read
	// to know exactly the word length.
	if( incomingWordPos == 0 )
	{
		if( c >= 0xF8 )
		{
			setComError( ControlByteReceived );
			closeCom(true);
		}
		if( (c & 0xF0) == 0xF0 )
		{
			setComError( ControlByteReceived );
			closeCom(true);
		}
		if( (c & 0xE0) == 0xE0 )
		{
			incomingWordSize = 4;
			c &= ~0xE0;
		}
		else
		if( (c & 0xC0) == 0xC0 )
		{
			incomingWordSize = 3;
			c &= ~0xC0;
		}
		else
		if( (c & 0x80) == 0x80 )
		{
			incomingWordSize = 2;
			c &= ~0x80;
		}
		else
			incomingWordSize = 1;
	}
	wordCountBuf[incomingWordPos] = c;
	if( incomingWordSize == ++incomingWordPos )
	{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		((char*)&incomingWordCount)[0] = wordCountBuf[0];
		((char*)&incomingWordCount)[1] = wordCountBuf[1];
		((char*)&incomingWordCount)[2] = wordCountBuf[2];
		((char*)&incomingWordCount)[3] = wordCountBuf[3];
#else
		((char*)&incomingWordCount)[0] = wordCountBuf[3];
		((char*)&incomingWordCount)[1] = wordCountBuf[2];
		((char*)&incomingWordCount)[2] = wordCountBuf[1];
		((char*)&incomingWordCount)[3] = wordCountBuf[0];
#endif
		return 1;
	}
	return 0;
}

/**
 * @brief Comm::receiveWord
 * Reads word from socket.
 * This functions uses incomingWordCount (filled by receiveWordCount)
 * to know how many bytes needs to read from socket.
 * This functions mus be called until incomingWordCount == incomingWord.count()
 * Here there is no control if socket is closed. There is no problem with this.
 * But, we must check if socket is open before calling this function.
 * @return bytes readed from socket.
 */
int Comm::receiveWord()
{
	int remain = incomingWordCount-incomingWord.count();

	Q_ASSERT_X(remain > 0, "receiveWord()", "negative word count remain calculated!");

	if( remain )
	{
		QByteArray tmp = m_sock.read(remain);
		if( tmp.count() )
			incomingWord.append(tmp);
		return tmp.count();
	}
	return 0;
}

/**
 * @brief Comm::receiveSentence
 * Slot called when data is ready to be read from socket connected to ROS.
 * This function fills up an internal QSentence struct. Once QSentence is
 * filled, at begining, this function is used to login. When login is done,
 * this function emits "comReceive(QSentence)" to allow application to
 * do his job with the sentence sended by ROS.
 */
void Comm::receiveSentence()
{
	while( m_sock.state() == QAbstractSocket::ConnectedState )
	{
		if( incomingWordCount == -1 )
			if( receiveWordCount() <= 0 )
				break;

		if( incomingWordCount > incomingWord.count() )
			if( !receiveWord() )
				break;

		// Check if the word is all received.
		if( incomingWordCount == incomingWord.count() )
		{
			// If == 0 means "end of sentence"
			// So, we need to process it.
			// If we are not loged into router, we'll try it.
			// Otherwise, we emit comReceive to let app do his job.
			if( incomingWordCount == 0 )
			{
				if( m_loginState != LogedIn )
					doLogin();
				else
				{
					emit comReceive(incomingSentence);
					resetSentence();
				}
			}
			else
			{
				incomingSentence.addWord(QString::fromLatin1(incomingWord));
				resetWord();
			}
		}
	}
}

/**
 * @brief ROS::Comm::connectTo
 * Starts connection to ROS at the addres addr and port.
 * this function does nothing if socket state is not on "UnconnectedState".
 * @param addr The addres where the ROS is. Can be a URL.
 * @param port The port where the ROS API is listening.
 */
void ROS::Comm::connectTo(const QString &addr, quint16 port)
{
	if( m_sock.state() == QAbstractSocket::UnconnectedState )
	{
		setComError( NoCommError );
		m_sock.connectToHost(m_addr = addr, m_port = port);
	}
}

/**
 * @brief Comm::closeCom
 * Closes connection to ROS.
 * If force is true, the connection will be closed inmediatly and
 * pending data on socket buffer will be discarded. If false, the
 * close could take a variable time because tries to close gracefully
 * and sends all remaining data in out socket buffer.
 * @param force (optional, defaults: false) True if you want/need to
 * force the closing.
 */
void Comm::closeCom(bool force)
{
	if( isConnected() || isConnecting() )
	{
		if( force || isConnecting() )
		{
			m_sock.abort();
			m_sock.close();
		}
		else
			m_sock.disconnectFromHost();
	}
	resetSentence();
}
/**
 * @brief Comm::doLogin
 * Tries to log into ROS.
 * It uses a MD5 encoding process using the challenge reported by ROS.
 * If there is some error, lastError is set and connection is closed.
 * This function will be called when data is present into socket until
 * a succefull login is performed.
 */
void Comm::doLogin()
{
	switch( m_loginState )
	{
	case NoLoged:
		resetSentence();
		break;
	case LoginRequested:
	{
		if( incomingSentence.getResultType() != QSentence::Done )
		{
			setComError( LoginRefused ) ;
			closeCom();
			break;
		}
		if( incomingSentence.attributes().count() != 1 )
		{
			setComError( LogingSentenceEmpty ) ;
			closeCom();
			break;
		}
		if( !incomingSentence.attribute("ret").count() )
		{
			setComError( LogingSentenceNoRet );
			closeCom();
			break;
		}
		if( incomingSentence.attribute("ret").count() != 32 )
		{
			setComError( LogingSentenceRet32 );
			closeCom();
			break;
		}

		emit loginRequest(&m_Username, &m_Password);

		sendSentence("/login", false,
							 QStringList() << QString("=name=%1").arg(m_Username)
										<< QString("=response=00%1").arg(QMD5::encode(m_Password, incomingSentence.attribute("ret"))));
		resetSentence();
		setLoginState(UserPassSended);
		break;
	}
	case UserPassSended:
		if( incomingSentence.getResultType() == QSentence::Done )
		{
			resetSentence();
			setLoginState(LogedIn);
		}
		else
		{
			setComError( LogingBadUsername );
			closeCom();
		}
		break;
#ifdef QT_DEBUG
	case LogedIn:
		Q_ASSERT_X( 0, "doLogin()", "Trying to login when we are already loged" );
		break;
#endif
	}
}

/**
 * @brief Comm::setLoginState
 * Sets login state and emits loginStateChanged when it changes.
 * @param s the new state to set.
 */
void Comm::setLoginState(Comm::LoginState s)
{
	if( s != m_loginState )
		emit loginStateChanged(m_loginState = s);
}

/**
 * @brief Comm::sendWordCount
 * Writes a word length on socket.
 * This funcion must be called before sending a word.
 * @param wordCount The word length to write to socket.
 */
void Comm::sendWordCount(int wordCount)
{
	char countBuff[4];

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
	countBuff[0] = ((char*)&wordCount)[0];
	countBuff[1] = ((char*)&wordCount)[1];
	countBuff[2] = ((char*)&wordCount)[2];
	countBuff[3] = ((char*)&wordCount)[3];
#else
	countBuff[0] = ((char*)&wordCount)[3];
	countBuff[1] = ((char*)&wordCount)[2];
	countBuff[2] = ((char*)&wordCount)[1];
	countBuff[3] = ((char*)&wordCount)[0];
#endif

	// write 1 byte
	if( wordCount < 0x80 )
		m_sock.write( countBuff, 1 );
	else
	if( wordCount < 0x4000 )		// write 2 bytes
	{
		countBuff[0] |= 0x80;
		m_sock.write( countBuff, 2 );
	}
	else
	if( wordCount < 0x200000)		// write 3 bytes
	{
		countBuff[0] |= 0xc0;
		m_sock.write( countBuff, 3 );
	}
	else
	if( wordCount < 0x10000000 )	// write 4 bytes (untested)
	{
		countBuff[0] |= 0xe0;
		m_sock.write( countBuff, 4 );
	}
	else
	{
		setComError( WordToSendTooLong );
		closeCom(true);
	}
}

/**
 * @brief ROS::Comm::onError
 * Slot connected to socket error signal.
 * This slot emits comError with a readable error string.
 */
void ROS::Comm::onSocketError(QAbstractSocket::SocketError err)
{
	emit comError(NoCommError, err);
}

/**
 * @brief Comm::onSocketStateChanges
 * Slot connected to stateChanged socket signal.
 * This function emits the new state of the socket
 * using internal enum.
 * @param s The new socket state.
 */
void Comm::onSocketStateChanges(QAbstractSocket::SocketState s)
{
	switch( s )
	{
	case QAbstractSocket::UnconnectedState:
		setLoginState(NoLoged);
		emit comStateChanged(Unconnected);
		return;
	case QAbstractSocket::HostLookupState:
		emit comStateChanged(HostLookup);
		return;
	case QAbstractSocket::ConnectingState:
		emit comStateChanged(Connecting);
		return;
	case QAbstractSocket::ConnectedState:
		emit comStateChanged(Connected);
		setLoginState(NoLoged);
		resetSentence();
		sendSentence( QSentence("/login"), false );
		setLoginState(LoginRequested);
		return;
	case QAbstractSocket::BoundState: // Este estado sólo se da en caso de ser un servidor.
		return;
	case QAbstractSocket::ClosingState:
		emit comStateChanged(Closing);
		return;
	case QAbstractSocket::ListeningState: // Este estado sólo se da en caso de ser un servidor.
		return;
	}
}
