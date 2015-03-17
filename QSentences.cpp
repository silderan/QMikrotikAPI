#include "QSentences.h"

using namespace Mkt;

void QSentence::Tokenize(QString &str, QStringList &tokens, const QString &delimiters)
{
    tokens.append( str.split(delimiters) );
}

void QSentence::getMap(QSentenceMap &sentenceMap)
{
	for( int i = 0; i < count(); ++i )
	{
		QStringList lista = at(i).split("=");
		// Aquí se ignora el primer elemento que siempre está vacío
		// Una respuesta del servidor es algo como "!done=ret=asfasfd"
		// así que la función "split" siempre genera un primer elemento vacío.
		for( int l = 1; l < lista.count(); l+=2 )
			sentenceMap.insert(lista[l], lista[l+1]);
	}
}

void QSentence::print() const
{
#ifdef QT_DEBUG
    printf("Sentence Word Count = %d\n", count());
    printf("Sentence returnType = %d\n", returnType);
#endif

    for( int i = 0; i < count(); ++i )
        printf("%s\n", at(i).toLatin1().data());

    printf("\n");
}


void QBlock::print() const
{
#ifdef QT_DEBUG
    printf("PrintBlock\n");
    printf("Block Size = %d\n", count());
#endif
    for( int i = 0; i < count(); ++i )
        at(i).print();
}
