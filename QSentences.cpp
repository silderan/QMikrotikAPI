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

#include "QSentences.h"

using namespace ROS;

/**
 * @brief splitWord
 * Splits incoming word into two parts.
 * This functions searchs for the first '=' character and
 * assings previous string into "name" variable and next one into
 * "value" variable.
 * This function is completly for internal use.
 * @param word The incoming word to split.
 * @param name Varible to store "name" part of word.
 * @param value Variable to store "value" part of word.
 * @param from First index of word string to search. This is used to
 * avoid parsing the first character from word that usually is
 * a special one like '.', '!', '=', etc...
 * @return true or false if was splited or not.
 * Non splited one stills stores the correct name part of word into
 * "name" variable. But, "value" variable remains untouched.
 */
bool splitWord(const QString &word, QString &name, QString &value, int from)
{
	int p = word.indexOf('=', from);
	if( p != -1 )
	{
		name = word.mid(from, p-from);
		value = word.right(word.count()-p-1);
		return true;
	}
	else
	{
		name = word.right(from);
		return false;
	}
}

/**
 * @brief QBasicAttrib::toWords
 * Creates a list of words.
 * This function uses the function "toWord" internally to make conversion.
 * It's used to create the correct word to be sent to ROS.
 * @return A list of strings representing words.
 */
QStringList QBasicAttrib::toWords() const
{
	QStringList rtn;

	foreach( QString key, keys() )
		rtn.append(toWord(key));

	return rtn;
}

/**
 * @brief QBasicAttrib::toWord
 * Creates a ROS-word usable string using
 * the name provided and the value asociated with it
 * in the internal mapping.
 * To create it, it uses also the "first char" provided
 * with constructor. This "first char" must be a attribute
 * valid char as '=' (for normal attributes) or '.' for (API
 * attributes)
 * @param name The name of the word to create the ROS-word.
 * @return The ROS-word created. If name doesn't exist into
 * mapping, empty string is returned.
 * Be carefull because if you use what this function returns
 * without ensuring that "name" is into mapping, the empty
 * word returned shall invalidate all sentence as empty word
 * means "sentence end" to ROS.
 */
QString QBasicAttrib::toWord(const QString &name) const
{
	QString val = value(name);
	if( !val.isEmpty() )
		return QString("%1%2=%3").arg(firstCh).arg(name,val);
	if( name.startsWith('!') )
		return QString("%1%2").arg(firstCh).arg(name);
	return QString();
}

/**
 * @brief QBasicAttrib::addWord
 * Inserts a new par name-value into internal mapping.
 * Note that internal mapping doesn't allow duplicate names. Si
 * inserting a name that already exists, will replace value instead
 * of append a new one.
 * @param name The name of the attribute.
 * @param value The value of attribute. Can be empty.
 * @see addWords
 * @see toWords
 */
void QBasicAttrib::addWord(const QString &name, const QString &value)
{
	insert(name, value);
}

/**
 * @brief QBasicAttrib::addWords
 * Appends a list of words.
 * Each word into list will be passed to addWord
 * function to parse it.
 * @param words List of strings representing words.
 * @see addWord
 * @see toWords
 */
void QBasicAttrib::addWords(const QStringList &words)
{
	foreach( QString word, words )
		addWord(word);
}

/**
 * @brief QBasicAttrib::addWord
 * Adds a word (a pair of name/value for attribute)
 * word parameter will be parsed by splitWord function.
 * If firs char stored in class is the same as first
 * char in word parameter, it will be ignored.
 * For example. If first char in class is '='
 * and word parameter is "=xxxx=yyyyy"
 * the firs '=' character is ignored, attribute name
 * will be "xxxx" and attribute value "yyyyy"
 * @param word The word to be analized to get attribute
 * name-value pair.
 */
void QBasicAttrib::addWord(const QString &word)
{
	if( word.count() < 3 )
		return;

	int from = (word[0].toLatin1() == firstCh) ? 1 : 0;
	QString name;
	QString value;
	splitWord(word, name, value, from);
	addWord(name, value);
}

/**
 * @brief QQuery::toWord
 * Creates a ROS-word string representation of the query.
 * It can be used to send directly to ROS
 * @return a ROS-like-word of the query.
 * @see QQuery::fromWord(const QString word)
 */
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

/**
 * @brief QQuery::fromWord
 * Parses word parameter to configure variable members of query struct
 * For example: words starting with '-' will be DontHasProp,
 * with '=' will be EqualProp, with '>' will be GreaterThanProp, etc...
 * This function may not be usefull as ROS won't reply query words.
 * But, you can do it if you like allowing user be able to write query
 * words directly, you can use this function to parse users input.
 * @param word The word to be parsed.
 * @return a reference of class itself.
 * @see QQuery::toWord()
 */
QQuery &QQuery::fromWord(const QString &word)
{
	if( word.count() < 2 )
		return *this;

	int from = 0;
	if( word[0].toLatin1() == '?' )
		from = 1;

	switch( word[from].toLatin1() )
	{
	default:
		if( splitWord(word, name, value, from) )
			type = EqualProp;
		else
			type = HasProp;
		break;
	case '-':
		type = DontHasProp;
		name = word.right(from);
		break;
	case '=':
		splitWord(word, name, value, from);
		type = EqualProp;
		break;
	case '>':
		splitWord(word, name, value, from);
		type = GreaterThanProp;
		break;
	case '<':
		splitWord(word, name, value, from);
		type = LessThanProp;
		break;
	case '#':
		name = word.right(from);
		type = Operation;
		break;
	}
	return *this;
}

/**
 * @brief QQueries::addQuery
 * Adds a query into list.
 * @param query the reference to the query to add into list.
 */
void QQueries::addQuery(const QQuery &query)
{
	append(query);
}

/**
 * @brief QQueries::addQuery
 * Adds a query-word into list.
 * This funcions calls a QQuery constructor (that calls fromWord internally)
 * to parse word.
 * @param word
 * @see QQuery::fromWord()
 */
void QQueries::addQuery(const QString &word)
{
	addQuery(QQuery(word));
}

/**
 * @brief QQueries::addQuery
 * Adds a query using name, value and query-type information.
 * This function calls internally a QQuery constructor.
 * @param name The name of query.
 * @param value The value of the query. Can be empty.
 * @param t The query type.
 */
void QQueries::addQuery(const QString &name, const QString &value, QQuery::Type t)
{
	addQuery(QQuery(name, value, t));
}

/**
 * @brief QQueries::toWords
 * Creates a list of ROS-like-words strings representing que queries.
 * This list can be used to send queries to ROS.
 * @return the ROS-like-words string list.
 * @see QQueries::addQueries()
 */
QStringList QQueries::toWords() const
{
	QStringList rtn;
	for( int i = 0; i < count(); i++ )
		rtn.append(at(i).toWord());
	return rtn;
}

/**
 * @brief QQueries::addQueries
 * Appeds queries from any alement of list.
 * Each element is parsed using addQuery function.
 * @param queries The list of ROS-like-word strings.
 * @see QQueries::toWords()
 */
void QQueries::addQueries(const QStringList &queries)
{
	foreach( QString query, queries)
		addQuery(query);
}

/**
 * @brief QSentence::toString
 * This funcions creates a one-lined string representing the sentence.
 * It includes command, attributes, API attributes and queries.
 * This string is only usefull for debuging purposes as cannot be
 * parsed to fillup again QSentence class.
 * @return A printable string representation of QSentence configuration.
 */
QString QSentence::toString() const
{
	QString rtn(m_cmd + resultString() + " " +
				attributes().toWords().join("") +
				APIattributes().toWords().join("") +
				queries().toWords().join(""));

	if( m_tag.count() )
		rtn.append(QString(".tag=%1").arg(m_tag));
	return rtn;
}

/**
 * @brief QSentence::resultString
 * Returns the result printable string.
 * @return the result printable string.
 */
QString QSentence::resultString() const
{
	switch( getResultType() )
	{
	case None:	return "";
	case Done:	return "!done";
	case Trap:	return "!trap";
	case Fatal:	return "!fatal";
	case Reply:	return "!re";
	default:	return "<error>";
	}
}

/**
 * @brief QSentence::addWord
 * Adds a word to the sentence. This word can be any of the valid words
 * allowed by QSentence (attributes, APIAttributes and queries)
 * Any word will be analized to know if it is a standard attribute, an API
 * attribute or a query. This parsing is up to first char of the word as
 * follow:
 * = will be a standard attribute.
 * . will be an API attribute.
 * / will be a command.
 * ! will be a result.
 * ? will be a query.
 * Result type word will set the resultType variable member class.
 * This function can be used to directly parse incoming word from ROS.
 * @param word The ROS-like word to parse.
 */
void QSentence::addWord(const QString &word)
{
	if( word.count() )
	{
		switch(word[0].toLatin1())
		{
		case '=':
			if( word.startsWith("=.id") )
				setID(word.right(word.count()-5));
			else
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
				setResultType(Done);
			else
			if( word == "!trap" )
				setResultType(Trap);
			else
			if( word == "!fatal" )
				setResultType(Fatal);
			else
			if( word == "!re" )
				setResultType(Reply);
			else
				throw "Unknown response type";
			break;
		case '/':
			setCommand(word);
			break;
		case '?':
			queries().append(QQuery(word));
			break;
		}
	}
}
