#ifndef QSENTENCES_H
#define QSENTENCES_H

#include <QMap>
#include <QStringList>

namespace ROS
{

class QBasicAttrib : public QMap<QString, QString>
{
	char firstCh;
public:
	QBasicAttrib(char c, const QStringList &words = QStringList()) : firstCh(c)
	{
		addWords(words);
	}
	inline void addAttribute(const QString &name, const QString &value)  { (*this)[name] = value; }
	inline QString attribute(const QString &name) const { return (*this)[name]; }
	QStringList toWords() const;
	QString toWord(const QString &name)const;
	void addWord(const QString &word);
	void addWord(const QString &name, const QString &value);
	void addWords(const QStringList &words);
};

struct QQuery
{
	enum Type
	{
		HasProp,
		DontHasProp,
		EqualProp,
		GreaterThanProp,
		LessThanProp,
		Operation
	};
	Type type;
	QString name;
	QString value;
	QQuery()
		: type(HasProp)
	{ }
	QQuery(const QString &propName)
		: type(HasProp), name(propName)
	{ }
	QQuery(const QString &propName, const QString &propVal, Type t = EqualProp)
		: type(t), name(propName), value(propVal)
	{ }
	bool operator==(const QQuery &q)
	{
		return (type == q.type) && (name == q.name) && (value == q.value);
	}
	bool operator!=(const QQuery &q) { return !(this->operator ==(q)); }
	QString toWord()const;
	QQuery &fromWord(const QString &word);
};

class QQueries : public QList<QQuery>
{
public:
	void addQuery(const QQuery &query);
	void addQuery(const QString &word);
	void addQuery(const QString &name, const QString &value, QQuery::Type t = QQuery::EqualProp);
	QStringList toWords() const;
	void addQueries(const QStringList &queries);
	QQueries(const QStringList &queries)
	{
		addQueries(queries);
	}
};

class QSentence : public QStringList
{
public:
	enum Result
    {
        None = 0,
        Done = 1,
        Trap = 2,
		Fatal = 3,
		Reply = 4
	};

private:
	Result resultType;		// Sentence return type.
	QString m_cmd;				// Sentence command.
	QString m_tag;				// Sentence tag (if any)
	QBasicAttrib m_Attributes;	// Attributes mapping.
	QBasicAttrib m_APIAttributes;//API Attributes mapping.
	QQueries m_Queries;			// Queries list.

public:
	QSentence(const QString &cmd = QString(), const QString &tag = QString(),
			  const QStringList &attribs = QStringList(),
			  const QStringList &APIAtts = QStringList(),
			  const QStringList &Queries = QStringList() )
		: m_cmd(cmd), m_tag(tag),
		  m_Attributes('=', attribs),
		  m_APIAttributes('.', APIAtts),
		  m_Queries(Queries)
	{
	}

	inline void setResultType(Result r) { resultType = r; }
	inline Result getResultType() const { return resultType; }
	QString toString() const;

	void clear()
	{
		QStringList::clear();
		m_cmd.clear();
		m_tag.clear();
		m_Attributes.clear();
		m_APIAttributes.clear();
		m_Queries.clear();
		resultType = None;
	}
	inline void setCommand(const QString &cmd) { m_cmd = cmd; }
	inline const QString &command() const { return m_cmd; }

	inline const QBasicAttrib &attributes() const { return m_Attributes; }
	inline QBasicAttrib &attributes() { return m_Attributes; }

	inline const QBasicAttrib &APIattributes() const { return m_APIAttributes; }
	inline QBasicAttrib &APIattributes() { return m_APIAttributes; }

	inline void setTag(const QString &tagname) { m_tag = tagname; }
	inline const QString &tag() const { return m_tag; }

	inline QQueries &queries() { return m_Queries; }
	inline const QQueries &queries() const { return m_Queries; }

	void addWord(const QString &word);
};
}
#endif // QSENTENCES_H
