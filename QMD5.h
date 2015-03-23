#ifndef QMD5_H
#define QMD5_H

#include <QString>

class QMD5
{
	static char hexToChar(char c);
	static char hexToChar(char low, char hi);
	static QByteArray hexToChars(const QString &s);
	static char charToHex(unsigned char c);
	static QString charsToHex(const QByteArray &s);

public:
	static QString encode(const QString &pass, const QString &seed);
};

#endif // QMD5_H
