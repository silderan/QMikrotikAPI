#ifndef QINIFILE_H
#define QINIFILE_H

#include <QMap>

typedef QMap<QString, QString> QIniData;

class QIniFile
{
public:
	static void load(const QString &file, QIniData *data);
	static void save(const QString &file, const QIniData &data);
};

#endif // QINIFILE_H
