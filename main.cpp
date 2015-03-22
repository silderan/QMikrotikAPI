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
