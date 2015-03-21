#include "QSentences.h"

using namespace ROS;

bool splitWord(const QString &word, QString &name, QString &value, int from)
{
	int p = word.indexOf('=', from);
	if( p != -1 )
	{
		name = word.mid(from, p-1);
		value = word.right(word.count()-p-1);
		return true;
	}
	else
	{
		name = word.right(from);
		return false;
	}
}

QStringList QBasicAttrib::toWords() const
{
	QStringList rtn;

	foreach( QString key, keys() )
		rtn.append(toWord(key));

	return rtn;
}

QString QBasicAttrib::toWord(const QString &name) const
{
	QString val = value(name);
	if( !val.isEmpty() )
		return QString("%1%2=%3").arg(firstCh).arg(name,val);
	return QString();
}

void QBasicAttrib::addWord(const QString &name, const QString &value)
{
	insert(name, value);
}

void QBasicAttrib::addWords(const QStringList &words)
{
	foreach( QString word, words )
		addWord(word);
}

void QBasicAttrib::addWord(const QString &word)
{
	if( word.count() < 3 )
		return;

	int from = word[0].toLatin1() == firstCh ? 1 : 0;
	QString name;
	QString value;
	splitWord(word, name, value, from);
	addWord(name, value);
}

QString QQuery::toWord() const
{
	switch( type )
	{
	case HasProp:			return QString("?%1").arg(name);
	case DontHasProp:		return QString("?-%1").arg(name);
	case EqualProp:			return QString("?=%1=%2").arg(name).arg(value);
	case GreaterThanProp:	return QString("?>%1=%2").arg(name).arg(value);
	case LessThanProp:		return QString("?<%1=%2").arg(name).arg(value);
	case Operation:			return QString("?#%1").arg(name);
	}
	return QString();
}

QQuery &QQuery::fromWord(const QString &queryString)
{
	if( queryString.count() < 3 )
		return *this;

	int from = 0;
	if( queryString[0].toLatin1() == '?' )
		from = 1;

	switch( queryString[from].toLatin1() )
	{
	default:
		if( splitWord(queryString, name, value, from) )
			type = EqualProp;
		else
			type = HasProp;
		break;
	case '-':
		type = DontHasProp;
		name = queryString.right(from);
		break;
	case '=':
		splitWord(queryString, name, value, from);
		type = EqualProp;
		break;
	case '>':
		splitWord(queryString, name, value, from);
		type = GreaterThanProp;
		break;
	case '<':
		splitWord(queryString, name, value, from);
		type = LessThanProp;
		break;
	case '#':
		name = queryString.right(from);
		type = Operation;
		break;
	}
	return *this;
}


void QQueries::addQuery(const QQuery &query)
{
	append(query);
}

void QQueries::addQuery(const QString &name)
{
	addQuery(QQuery(name));
}

void QQueries::addQuery(const QString &name, const QString &value, QQuery::Type t)
{
	addQuery(QQuery(name, value, t));
}

QStringList QQueries::toWords() const
{
	QStringList rtn;
	for( int i = 0; i < count(); i++ )
		rtn.append(at(i).toWord());
	return rtn;
}

void QQueries::addQueries(const QStringList &queries)
{
	foreach( QString query, queries)
		addQuery(query);
}

QString QSentence::toString() const
{
	QString rtn(m_cmd +
				attributes().toWords().join("") +
				APIattributes().toWords().join("") +
				queries().toWords().join(""));

	if( m_tag.count() )
		rtn.append(QString(".tag=%1").arg(m_tag));
	return rtn;
}

void QSentence::addWord(const QString &word)
{
	if( word.count() )
	{
		switch(word[0].toLatin1())
		{
		case '=':
			attributes().addWord(word);
			break;
		case '.':
			if( word.startsWith(".tag") )
				setTag(word.right(word.count()-5));
			else
				APIattributes().addWord(word);
			break;
		case '!':
			if( word == "!done" )
				setReturnType(Done);
			else
			if( word == "!trap" )
				setReturnType(Trap);
			else
			if( word == "!fatal" )
				setReturnType(Fatal);
			else
			if( word == "!re" )
				setReturnType(Reply);
			else
				throw "Unknown response type";
			break;
		case '/':
			setCommand(word);
			break;
		case '?':
			queries().append(QQuery().fromWord(word));
			break;
		}
	}
}
