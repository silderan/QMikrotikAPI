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
		Reply = 4,
		Partial = 5	// Aún están llegando datos.
    };
	enum QueryType
	{
		HasProp,
		DontHasProp,
		EqualProp,
		GreaterThanProp,
		LessThanProp,
		Operation
	};
	QString m_cmd;
	QString m_tag;
	QMap<QString, QString> m_Attributes;
	QMap<QString, QString> m_APIAttributes;
	QMap<QString, QString> m_Queries;

private:
    ReturnType returnType;     // return type of sentence

public:
	inline void setReturnType(ReturnType r) { returnType = r; }
    inline ReturnType getReturnType() const { return returnType; }
	QString toString() const;

    void getMap(QSentenceMap &sentenceMap);

	void clear()
	{
		QStringList::clear();
		m_cmd.clear();
		m_tag.clear();
		m_Attributes.clear();
		m_APIAttributes.clear();
		m_Queries.clear();
		returnType = None;
	}
	inline void setCommand(const QString &cmd) { m_cmd = cmd; }
	inline const QString &command() const { return m_cmd; }

	inline void addAttribute(const QString &name, const QString &value)
	{ m_Attributes[name] = value; }
	inline QString attribute(const QString &name) const { return m_Attributes[name]; }
	inline QStringList attributes() const { return m_Attributes.values(); }
	inline int attributesCount() const { return m_Attributes.count(); }

	inline void addAPIAttribute(const QString &name, const QString &value)
	{ m_APIAttributes[name] = value; }
	inline QString APIattribute(const QString &name) const { return m_APIAttributes[name]; }
	inline QStringList APIAttributes() const { return m_APIAttributes.values(); }
	inline int APIattributesCount() const { return m_APIAttributes.count(); }

	inline void setTag(const QString &tagname) { m_tag = tagname; }
	inline QString tag() const { return m_tag; }

	inline void addQuery(const QString &name, const QString &value) { m_Queries[name] = value; }
	inline QString query(const QString &name) const { return m_Queries[name]; }
	inline QStringList queries() const { return m_Queries.values(); }
	inline int queriesCount() const { return m_Queries.count(); }

	void addQuery(QueryType t, const QString &name, const QString &value);
	void addWord(const QString &word);
};
}
#endif // QSENTENCES_H
