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

#include "mainwindow.h"
#include <QApplication>
#include <QCryptographicHash>

#include <string>
#include <vector>

#include "Comm.h"
using namespace std;

void Tokenize(const string &str, vector<string> &tokens, const string &delimiters)
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
    MainWindow w;
//	QByteArray ori = "65660F0F0F0F0F0F0F0F0F0F0F0F0F0F";
//	QByteArray ch1 = QMD5::ToBinary(QString::fromLatin1(ori)).toLatin1();
//	QByteArray ch2 = QMD5::hexToChars(ori);
//	QString or1 = QMD5::charsToHex(ch1);
//	QString or2 = QMD5::DigestToHexString((md5_byte_t*)ch1.data());
//	QString My = QMD5::MyEncode("hola", ori);
//	QString Th = QMD5::ThEncode("hola", ori);
//	QString Ps = QMD5::encode("hola", ori);
	w.show();

    return a.exec();
}
