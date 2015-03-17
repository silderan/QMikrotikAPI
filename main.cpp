#include "mainwindow.h"
#include <QApplication>
#include <QCryptographicHash>

#include <string>
#include <vector>

#include "APICom.h"
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

/*    md5_state_t state;
    md5_byte_t out[16];
    char hola[5] = "hola";
    QMD5::init(&state);
    QMD5::append(&state, (md5_byte_t*)hola, 4);
    QMD5::finish(&state, out);

    QCryptographicHash h(QCryptographicHash::Md5);
    h.addData(hola,4);
    QByteArray b = h.result();
*/
	w.show();

    return a.exec();
}
