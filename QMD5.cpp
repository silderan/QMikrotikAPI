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

#include "QMD5.h"

char QMD5::hexToChar(char c)
{
	switch( c )
	{
	case 'A': case 'a': return 10;
	case 'B': case 'b': return 11;
	case 'C': case 'c': return 12;
	case 'D': case 'd': return 13;
	case 'E': case 'e': return 14;
	case 'F': case 'f': return 15;
	default: return c - '0';
	}
}

char QMD5::hexToChar(char low, char hi)
{
	return hexToChar(low) + (hexToChar(hi)*16);
}

/**
 * @brief QMD5::hexToChars
 * Converts a string with a 16 hex pair numbers representig chars value to a
 * array of chars. That is 6465666768 will become to "defgh"
 * This functions is needed becouse ROS returns 16 chars encoded
 * that way. Return is a QByteArray becouse it's more useful for
 * MD5 encoding.
 * @param s the 32 sized string with the 16 chars encoded.
 * @return The chars decoded.
 * @see charsToHex
 */
QByteArray QMD5::hexToChars(const QString &s)
{
	if( s.count() != 32 )
		return s.toLatin1();
	QByteArray rtn;
	rtn.resize(16);

	for( int i = 0; i < 32; i+=2 )
		rtn[i>>1] = hexToChar(s[i+1].toLatin1(), s[i].toLatin1());

	return rtn;
}

char QMD5::charToHex(unsigned char c)
{
	if( c <= 9 )
		return '0' + c;
	return 'A' + (c - 10);
}

/**
 * @brief QMD5::charsToHex
 * Converts a 16 sized array of chars into a 32 sized string with encoded chars
 * It's used to send to ROS the encoded MD5 password.
 * @param s the array of chars to convert. Must has 16 chars.
 * @return The hexadecimal pairs number representation of chars.
 */
QString QMD5::charsToHex(const QByteArray &s)
{
	char rtn[32 + 1];

	for( int i = 0; i < 16; ++i )
	{
		rtn[(i<<1)+1] = charToHex(s[i] & 0xF);
		rtn[i<<1] = charToHex((s[i]>>4) & 0xF);
	}
	rtn[33] = '\0';

	return QString::fromLatin1(rtn);
}

#include <QCryptographicHash>
QString QMD5::encode(const QString &pass, const QString &seed)
{
	QCryptographicHash encoder(QCryptographicHash::Md5);

	encoder.addData("", 1);
	encoder.addData(pass.toLatin1());
	encoder.addData(QMD5::hexToChars(seed));

	return QMD5::charsToHex(encoder.result());
}
