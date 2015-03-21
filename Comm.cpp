#include "Comm.h"

using namespace ROS;

Comm::Comm(QObject *papi)
 : QObject(papi), m_loginState(NoLoged), incomingWordSize(-1)
{
	connect( &m_sock, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(onError(QAbstractSocket::SocketError)) );
    connect( &m_sock, SIGNAL(hostFound()), this, SLOT(onHostLookup()) );
	connect( &m_sock, SIGNAL(connected()), this, SLOT(onConnected()) );

	connect( &m_sock, SIGNAL(readyRead()), this, SLOT(onReadyRead()) );
	connect( &m_sock, SIGNAL(disconnected()), this, SLOT(onDisconnected()) );
	connect( &m_sock, SIGNAL(readyRead()), this, SLOT(onReadyRead()) );
}

Comm::~Comm()
{
	m_sock.close();
}

/**
 * @brief Comm::writeSentence
 * Sends a full sentence.
 * If there is no tag provided in sentence info, a unique one
 * is created to send to and returned.
 * @param sent Sentence class with the info to sent to Router.
 * @return tag used for sentence.
 */
QString Comm::sendSentence(const QSentence &sent, bool bAddTag)
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

	if( bAddTag )
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

/********************************************************************
 * Read a word from the socket
 * The word that was read is returned as a string
 ********************************************************************/
int Comm::readWord()
{
	if( incomingWordSize <= 0 )
		switch( incomingWordSize = readLength() )
		{
		// Negative values means comm interrupted when reading length.
		case -1:// Nothing readed. No problem. Will be readed later.
			return -1;
		case -2:// Cannot read 2nd byte (TODO: Handle it)
		case -3:// Cannot read 3rd byte (TODO: Handle it)
		case -4:// Cannot read 4th byte (TODO: Handle it)
			throw "cannot read word length.";
			break;
		case 0:
			incomingWord.clear();
			return 0;
		}

	QByteArray tmp = m_sock.read(incomingWordSize);
	if( tmp.count() )
	{
		incomingWordSize -= tmp.count();
		incomingWord.append(tmp);
	}
	return incomingWordSize == 0 ? 1 : -1;
}

/********************************************************************
 * Read a Sentence from the socket
 * A Sentence struct is returned
 ********************************************************************/
void Comm::readSentence()
{
	int i;
	while( (i = readWord()) != 0 )
	{
		switch( i )
		{
		case 1:
			incomingSentence.addWord(incomingWord);
			incomingWord.clear();
			incomingWordSize = -1;
			break;
		case -1:
			bIncomingCompleted = false;
			return;
		}
	}
	bIncomingCompleted = true;
}

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

void Comm::doLogin()
{
	switch( m_loginState )
	{
	case NoLoged:
		break;
	case LoginRequested:
	{
		if( incomingSentence.getReturnType() != QSentence::Done )
		{
			emit comError(tr("Cannot login"));
			m_loginState = NoLoged;
			m_sock.close();
			break;
		}
		if( incomingSentence.attributes().count() != 1 )
		{
			emit comError(tr("Unknown remote login sentence format: didn't receive anything"));
			m_loginState = NoLoged;
			incomingSentence.clear();
			m_sock.close();
			break;
		}
		if( !incomingSentence.attributes().attribute("ret").count() )
		{
			emit comError(tr("Unknown remote login sentence format: Doesn't receive 'ret' namefield"));
			m_loginState = NoLoged;
			incomingSentence.clear();
			m_sock.close();
			break;
		}
		if( incomingSentence.attributes().attribute("ret").count() != 32 )
		{
			emit comError(tr("Unknown remote login sentence format: 'ret' field doesn't contains 32 characters"));
			m_loginState = NoLoged;
			incomingSentence.clear();
			m_sock.close();
			break;
		}

		md5_state_t state;
		md5_byte_t digest[16];

		////Place of interest: Check to see if this md5Challenge string works as as string.
		//   It may not because it needs to be binary.
		// convert szMD5Challenge to binary
		QString md5ChallengeBinary = QMD5::ToBinary(incomingSentence.attributes().attribute("ret"));

		// get md5 of the password + challenge concatenation
		QMD5::init(&state);
		QMD5::append(&state, (const md5_byte_t *)"", 1);
		QMD5::append(&state, (const md5_byte_t *)m_Password.toLatin1().data(),
								 strlen(m_Password.toLatin1().data()));
		QMD5::append(&state, (const md5_byte_t *)md5ChallengeBinary.toLatin1().data(), 16);
		QMD5::finish(&state, digest);

		// convert this digest to a string representation of the hex values
		// digest is the binary format of what we want to send
		// szMD5PasswordToSend is the "string" hex format
		QString md5PasswordToSend = QMD5::DigestToHexString(digest);

		// put together the login sentence
		sendWord("/login");
		sendWord(QString("=name=%1").arg(m_Username));
		sendWord(QString("=response=00%1").arg(md5PasswordToSend));
		sendWord("");

		incomingSentence.clear();
		incomingWordSize = -1;
		m_loginState = UserPassSended;
		break;
	}
	case UserPassSended:
		if( incomingSentence.getReturnType() == QSentence::Done )
		{
			m_loginState = LogedIn;
			incomingWordSize = -1;
			incomingSentence.clear();
			emit routerListening();
		}
		else
		{
			m_loginState = NoLoged;
			emit comError(tr("Invalid Username or Password"));
			emit comError(tr("remote msg: %1").arg(incomingSentence.attributes().attribute("message")));
			incomingWordSize = -1;
			incomingSentence.clear();
			m_sock.close();
		}
		break;
	case LogedIn:
		throw "router is loged already in";
		break;
	}
}

void Comm::writeLength(int messageLength)
{
    static char encodedLengthData[4];    // encoded length to send to the api socket
    char *lengthData;           // exactly what is in memory at &iLen integer

    // set cLength address to be same as messageLength
    lengthData = (char *)&messageLength;

    // write 1 byte
    if( messageLength < 0x80 )
    {
        encodedLengthData[0] = lengthData[0];

        m_sock.write( encodedLengthData, 1 );
    }
    else
    if( messageLength < 0x4000 )// write 2 bytes
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
    if( messageLength < 0x200000)// write 3 bytes
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
    if( messageLength < 0x10000000 ) // write 4 bytes (untested)
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
        printf("Length of word is %d\n", messageLength);
        printf("Word is too long.\n");
        throw "Word too long";
    }
}

/********************************************************************
 * Read a message length from the socket
 *
 * 80 = 10000000 (2 character encoded length)
 * C0 = 11000000 (3 character encoded length)
 * E0 = 11100000 (4 character encoded length)
 *
 * Message length is returned
 ********************************************************************/
// TODO: Esta función tiene un par de fallos en la
// asignación de memoria y se podría simplificar sin usar nada de ella.
int Comm::readLength()
{
	char firstChar;			// first character read from socket
	char *lengthData;		// length of next message to read...will be cast to int at the end
	int *messageLength;		// calculated length of next message (Cast to int)

	lengthData = (char *) calloc(sizeof(int), 1);

	if( !m_sock.read( &firstChar, 1 ) )
		return -1;

	// read 4 bytes
	// this code SHOULD work, but is untested...
	if( (firstChar & 0xE0) == 0xE0 )
	{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		lengthData[3] = firstChar;
		lengthData[3] &= 0x1f;        // mask out the 1st 3 bits
		if( !m_sock.read( &lengthData[2], 1 ) ) return -2;
		if( !m_sock.read( &lengthData[1], 1 ) ) return -3;
		if( !m_sock.read( &lengthData[0], 1 ) ) return -4;
#else
		lengthData[0] = firstChar;
		lengthData[0] &= 0x1f;        // mask out the 1st 3 bits
		if( !m_sock.read( &lengthData[1], 1 ) ) return -2;
		if( !m_sock.read( &lengthData[2], 1 ) ) return -3;
		if( !m_sock.read( &lengthData[3], 1 ) ) return -4;
#endif
		messageLength = (int *)lengthData;
	}
	else
	if( (firstChar & 0xC0) == 0xC0 ) // read 3 bytes
	{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		lengthData[2] = firstChar;
		lengthData[2] &= 0x3f;        // mask out the 1st 2 bits
		if( !m_sock.read( &lengthData[1], 1 ) ) return -2;
		if( !m_sock.read( &lengthData[0], 1 ) ) return -3;
#else
		lengthData[1] = firstChar;
		lengthData[1] &= 0x3f;        // mask out the 1st 2 bits
		if( !m_sock.read( &lengthData[2], 1 ) ) return -2;
		if( !m_sock.read( &lengthData[3], 1 ) ) return -3;
#endif
		messageLength = (int *)lengthData;
	}
	else
	if( (firstChar & 0x80) == 0x80) // read 2 bytes
	{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		lengthData[1] = firstChar;
		lengthData[1] &= 0x7f;        // mask out the 1st bit
		if( !m_sock.read( &lengthData[0], 1 ) ) return -2;
#else
		lengthData[2] = firstChar;
		lengthData[2] &= 0x7f;        // mask out the 1st bit
		if( !m_sock.read( &lengthData[3], 1 ) ) return -2;
#endif
		messageLength = (int *)lengthData;
	}
	else // assume 1-byte encoded length...same on both LE and BE systems
	{
		messageLength = (int *) malloc(sizeof(int));
		*messageLength = (int)firstChar;
	}

	int retMessageLength = *messageLength;
	delete messageLength;
	delete [] lengthData;

	return retMessageLength;
}

void ROS::Comm::onError(QAbstractSocket::SocketError /*err*/)
{
    if( m_sock.state() != QAbstractSocket::ConnectedState )
        emit comConnected(false);
	emit comError(m_sock.errorString());
}

void Comm::onConnected()
{
    emit comConnected(true);

	m_loginState = NoLoged;
	emit loginRequest(&m_Username, &m_Password);

	m_loginState = LoginRequested;
	incomingSentence.clear();
	//Send login message
	sendSentence( QSentence("/login"), false );
}

void Comm::onDisconnected()
{
	emit comConnected(false);
}

void Comm::onHostLookup()
{
	emit addrFound();
}

void Comm::onReadyRead()
{
	readSentence();
	if( bIncomingCompleted )
	{
		if( m_loginState != LogedIn )
			doLogin();
		else
		{
			emit comReceive(incomingSentence);
			incomingSentence.clear();
		}
		bIncomingCompleted = false;
	}
}
