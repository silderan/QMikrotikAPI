#ifndef QSENTENCES_H
#define QSENTENCES_H

#include <QMap>
#include <QStringList>

namespace Mkt
{

typedef QMap<QString, QString>  QSentenceMap;

class QSentence : public QStringList
{
public:
    enum ReturnType
    {
        None = 0,
        Done = 1,
        Trap = 2,
		Fatal = 3,
		Partial = 4	// Aún están llegando datos.
    };
private:
    ReturnType returnType;     // return type of sentence
    void Tokenize(QString &str, QStringList &tokens, const QString &delimiters = " ");

public:
    inline void setReturnType(ReturnType r) { returnType = r; }
    inline ReturnType getReturnType() const { return returnType; }

    void getMap(QSentenceMap &sentenceMap);
    void print() const;
	void clear()
	{
		QStringList::clear();
		returnType = None;
	}
};

class QBlock : public QList<QSentence>
{
public:
    void print() const;
};
}
#endif // QSENTENCES_H
