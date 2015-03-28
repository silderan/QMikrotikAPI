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
