#include "Comm.h"

using namespace ROS;

#include <QMessageBox>

Comm::Comm(QObject *papi)
 : QObject(papi), m_loginState(NoLoged), incomingWordSize(-1)
{
	connect( &m_sock, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(onError(QAbstractSocket::SocketError)) );
	connect( &m_sock, SIGNAL(readyRead()), this, SLOT(readSentence()) );
	connect( &m_sock, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
			 this, SLOT(onSocketStateChanges(QAbstractSocket::SocketState)) );
}

Comm::~Comm()
{
	m_sock.close();
}

void Comm::resetWord()
{
	incomingWord.clear();
	incomingWordSize = -1;
	incomingWordPos = 0;
	incomingWordCount = -1;
	(*(int*)wordCountBuf) = 0;
}

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
 * @param addTag (Optional, default==true) Tells function to use (or,
 * eventually create and use) a tag for sentence to sent.
 * @return tag used for sentence.
 */
QString Comm::sendSentence(const QSentence &sent, bool addTag)
{
	static int ID = 0;
	QString word;

	sendWord(sent.command());
	foreach( word, sent.attributes().toWords() )
		sendWord(word);
	foreach( word, sent.APIattributes().toWords() )
		sendWord(word);
	foreach( word, sent.queries().toWords() )
		sendWord(word);

	if( addTag )
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
 * @brief Comm::sendWord
 * Sends a word to router.
 * Word lenght is calculated and is sended before word itself.
 * @param word
 */
void Comm::sendWord(const QString &word)
{
	writeLength( word.count() );
	m_sock.write( word.toLatin1() );
}

int Comm::receiveWordCount()
{
	char c;
	int i = m_sock.read(&c, 1);
	if( i <= 0 )
		return i;

	if( incomingWordPos == 0 )
	{
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
 * @brief Comm::readWord
 * Reads a word from ROS socket.
 * At first, tries to read word length. After that, reads
 * the word itself.
 * When this functions returns -1 means that data is incompleted. So,
 * can be recalled again when data is avaiable on socket to complete
 * word reading.
 * TODO: If word length info arrived is incomplete, there is no code
 * to handle it and all will break up.
 * @return up to if word was readed or not:
 * -1: No data or incompleted readed from socket.
 * -2 to -4: word length info incompleted. TODO: handle it.
 * 0: Empty word readed.
 * 1: Word readed.
 */
int Comm::readWord()
{
	if( incomingWordCount == -1 )
	{
		int i = receiveWordCount();
		if( i <= 0 )
			return i;
	}
/*	if( incomingWordSize <= 0 )
		switch( incomingWordSize = readLength() )
		{
		// Negative values means comm interrupted when reading length.
		case -1:// Nothing readed. No problem. Will be readed later.
			return -1;
		case -2:// Cannot read 2nd byte (TODO: Handle it)
		case -3:// Cannot read 3rd byte (TODO: Handle it)
		case -4:// Cannot read 4th byte (TODO: Handle it)
			throw "Incomplete word length arrived.";
		case 0:
			// Empty word readed.
			incomingWord.clear();
			return 0;
		}
*/
	QByteArray tmp = m_sock.read(incomingWordSize);
	if( tmp.count() )
	{
		incomingWordSize -= tmp.count();
		incomingWord.append(tmp);
	}
	// If all word is readed, returns 1. Otherwise, -1 is returned.
	return incomingWordSize == 0 ? 1 : -1;
}

/**
 * @brief Comm::readSentence
 * Slot called when data is ready to be read from socket connected to ROS.
 * This function fills up an internal QSentence struct. Once QSentence is
 * filled, at begining, this function is used to login. When login is done,
 * this function emits "comReceive(QSentence)" to allow application to
 * do his job with the sentence sended by ROS.
 */
void Comm::readSentence()
{
	try
	{
	while( readWord() >= 0 )
	{
		if( m_sock.state() != QAbstractSocket::ConnectedState )
			break;
		if( incomingWord.isEmpty() )
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
			incomingSentence.addWord(incomingWord);
			resetWord();
		}
	}
	}
	catch( const char *err )
	{
		QMessageBox::warning(NULL, tr("reading sentence"), tr("Error reading data from socket: %1").arg(err));
		closeCom(true);
	}
}

/**
 * @brief ROS::Comm::connectTo
 * Starts connection to ROS at the addres addr and port.
 * If cannot starts connection, emits a comError signal
 * @param addr The addres where the ROS is. Can be a URL.
 * @param port The port where the ROS API is listening.
 * @return true if connection is started or false if not
 * Currently, false is returned in case that the socket is
 * not at UnconnectedState
 */
bool ROS::Comm::connectTo(const QString &addr, quint16 port)
{
	if( QAbstractSocket::UnconnectedState != m_sock.state() )
	{
		emit comError(tr("Trying to connect a allready opened socket"));
		return false;
	}

	m_sock.connectToHost(m_addr = addr, m_port = port);
	return true;
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
	if( isConnected() )
	{
		if( force )
		{
			m_sock.abort();
			m_sock.close();
			emit comStateChanged(Unconnected);
			emit comError(tr("forced abort/close on socket"));
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
 * If there is some error, comError signal will be emited and socket
 * will be closed inmediatly.
 * This function will be called when data is present into socket until
 * a succefull login is performed.
 */
void Comm::doLogin()
{
	switch( m_loginState )
	{
	case NoLoged:
		break;
	case LoginRequested:
	{
		if( incomingSentence.getResultType() != QSentence::Done )
		{
			emit comError(tr("Cannot login"));
			setLoginState(NoLoged);
			closeCom();
			break;
		}
		if( incomingSentence.attributes().count() != 1 )
		{
			emit comError(tr("Unknown remote login sentence format: didn't receive anything"));
			setLoginState(NoLoged);
			closeCom();
			break;
		}
		if( !incomingSentence.attributes().attribute("ret").count() )
		{
			emit comError(tr("Unknown remote login sentence format: Doesn't receive 'ret' namefield"));
			setLoginState(NoLoged);
			closeCom();
			break;
		}
		if( incomingSentence.attributes().attribute("ret").count() != 32 )
		{
			emit comError(tr("Unknown remote login sentence format: 'ret' field doesn't contains 32 characters"));
			setLoginState(NoLoged);
			closeCom();
			break;
		}

		emit loginRequest(&m_Username, &m_Password);
		// put together the login sentence
		sendWord("/login");
		sendWord(QString("=name=%1").arg(m_Username));
		sendWord(QString("=response=00%1").arg(QMD5::encode(m_Password, incomingSentence.attributes().attribute("ret"))));
		sendWord("");

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
			emit comError(tr("Invalid Username or Password"));
			emit comError(tr("remote msg: %1").arg(incomingSentence.attributes().attribute("message")));
			setLoginState(NoLoged);
			resetSentence();
		}
		break;
	case LogedIn:
		throw "router is loged already in";
		break;
	}
}

void Comm::setLoginState(Comm::LoginState s)
{
	if( s != m_loginState )
		emit loginStateChanged(m_loginState = s);
	else
		m_loginState = s;
}

/**
 * @brief Comm::writeLength
 * Writes a word length on socket.
 * This funcion must be called before sending word.
 * @param wordLength The word length to write to socket.
 */
void Comm::writeLength(int wordLength)
{
	static char encodedLengthData[4];	// encoded length to send to the api socket
	char *lengthData;					// exactly what is in memory at &wordLength integer

	// set cLength address to be same as messageLength
	lengthData = (char *)&wordLength;

	// write 1 byte
	if( wordLength < 0x80 )
	{
		encodedLengthData[0] = lengthData[0];

		m_sock.write( encodedLengthData, 1 );
	}
	else
	if( wordLength < 0x4000 )// write 2 bytes
	{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		encodedLengthData[0] = lengthData[1] | 0x80;
		encodedLengthData[1] = lengthData[0];
#else
		encodedLengthData[0] = lengthData[2] | 0x80;
		encodedLengthData[1] = lengthData[3];
#endif
		m_sock.write( encodedLengthData, 2 );
	}
	else
	if( wordLength < 0x200000)// write 3 bytes
	{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		encodedLengthData[0] = lengthData[2] | 0xc0;
		encodedLengthData[1] = lengthData[1];
		encodedLengthData[2] = lengthData[0];
#else
		encodedLengthData[0] = lengthData[1] | 0xc0;
		encodedLengthData[1] = lengthData[2];
		encodedLengthData[2] = lengthData[3];
#endif
		m_sock.write( encodedLengthData, 3 );
	}
	else
	if( wordLength < 0x10000000 ) // write 4 bytes (untested)
	{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		encodedLengthData[0] = lengthData[3] | 0xe0;
		encodedLengthData[1] = lengthData[2];
		encodedLengthData[2] = lengthData[1];
		encodedLengthData[3] = lengthData[0];
#else
		encodedLengthData[0] = lengthData[0] | 0xe0;
		encodedLengthData[1] = lengthData[1];
		encodedLengthData[2] = lengthData[2];
		encodedLengthData[3] = lengthData[3];
#endif
		m_sock.write( encodedLengthData, 4 );
	}
	else
	{   // this should never happen
		printf("Length of word is %d\n", wordLength);
		printf("Word is too long.\n");
		throw "Word too long";
	}
}

/**
 * @brief Comm::readLength
 * Reads a word length from socket.
 * @return the length readed.
 * @todo Esta función tiene un par de fallos en la
 * asignación de memoria y se podría simplificar sin usar nada de ella.
 */
int Comm::readLength()
{
	char firstChar;			// first character read from socket
	char lengthData[4];		// length of next message to read...will be cast to int at the end
	int *messageLength;		// calculated length of next message (Cast to int)

	if( m_sock.read(&firstChar, 1) != 1 )
		return -1;

	messageLength = (int *)lengthData;

	// 4 bytes.
	if( (firstChar & 0xE0) == 0xE0 )
	{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		lengthData[3] = firstChar & ~0xE0;
		if( m_sock.read(lengthData+2, 1) != 1 ) return -2;
		if( m_sock.read(lengthData+1, 1) != 1 ) return -3;
		if( m_sock.read(lengthData, 1) != 1 ) return -4;
#else
		lengthData[0] = firstChar & ~0xE0;
		if( m_sock.read(lengthData+1, 1) != 1 ) return -2;
		if( m_sock.read(lengthData+2, 1) != 1 ) return -3;
		if( m_sock.read(lengthData+3, 1) != 1 ) return -4;
#endif
	}
	else
	// 3 bytes.
	if( (firstChar & 0xC0) == 0xC0 )
	{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		lengthData[2] = firstChar & ~0xC0;
		if( m_sock.read(lengthData+1, 1) != 1 ) return -2;
		if( m_sock.read(lengthData+0, 1) != 1 ) return -3;
#else
		lengthData[1] = firstChar & ~0xC0;
		if( m_sock.read(lengthData+2, 1) != 1 ) return -2;
		if( m_sock.read(lengthData+3, 1) != 1 ) return -3;
#endif
	}
	else
	// 2 bytes.
	if( (firstChar & 0x80) == 0x80)
	{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		lengthData[1] = firstChar & ~0x80;
		if( m_sock.read(lengthData+0, 1) != 1 ) return -2;
#else
		lengthData[2] = firstChar & ~0x80;
		if( m_sock.read(lengthData+3, 1) != 1 ) return -2;
#endif
	}
	else
	// 1 byte.
		return (int)firstChar;

	return *messageLength;
}

/**
 * @brief ROS::Comm::onError
 * Slot connected to socket error signal.
 * This slot emits comError and comConnected apropiatelly.
 */
void ROS::Comm::onError(QAbstractSocket::SocketError /*err*/)
{
	emit comError(m_sock.errorString());
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
