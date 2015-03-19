#include "QSentences.h"

using namespace Mkt;

QString QSentence::toString() const
{
	QString rtn(m_cmd);
	if( m_Attributes.count() )
	{
		QMap<QString, QString>::const_iterator i;
		for( i = m_Attributes.constBegin(); i != m_Attributes.constEnd(); ++i )
			rtn.append(QString("=%1=%2").arg(i.key()).arg(i.value()));
	}
	if( m_APIAttributes.count() )
	{
		QMap<QString, QString>::const_iterator i;
		for( i = m_APIAttributes.constBegin(); i != m_APIAttributes.constEnd(); ++i )
			rtn.append(QString("=%1=%2").arg(i.key()).arg(i.value()));
	}
	if( m_Queries.count() )
	{
		QMap<QString, QString>::const_iterator i;
		for( i = m_Queries.constBegin(); i != m_Queries.constEnd(); ++i )
			rtn.append(QString("?%1=%2").arg(i.key()).arg(i.value()));
	}
	if( m_tag.count() )
		rtn.append(QString(".tag=%1").arg(m_tag));
	return rtn;
}

void QSentence::addQuery(QSentence::QueryType t, const QString &name, const QString &value)
{
	switch(t)
	{
	case HasProp:
		break;
	case DontHasProp:
		m_Queries[QString("-%1").arg(name)] = "";
		break;
	case EqualProp:
		m_Queries[QString("=%1").arg(name)] = value;
		break;
	case GreaterThanProp:
		m_Queries[QString(">%1").arg(name)] = value;
		break;
	case LessThanProp:
		m_Queries[QString("<%1").arg(name)] = value;
		break;
	case Operation:
		m_Queries[QString("#%1").arg(name)] = value;
		break;
	default:
		m_Queries[name] = value;
	}
}

void QSentence::addWord(const QString &word)
{
	if( word.count() )
	{
		int p;
		switch(word[0].toLatin1())
		{
		case '=':
			p = word.indexOf('=', 1);
			addAttribute(word.mid(1, p-1), word.right(word.count()-p-1));
			break;
		case '.':
			p = word.indexOf('=', 1);
			if( word.startsWith(".tag") )
				setTag(word.right(word.count()-p-1));
			else
				addAPIAttribute(word.mid(1, p-1), word.right(word.count()-p-1));
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
			p = word.indexOf('=', 1);
			addQuery(word.mid(1, p-1), word.right(word.count()-p-1));
			break;
		}
	}
}
