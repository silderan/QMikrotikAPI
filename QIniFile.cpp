#include "QIniFile.h"

#include <QFile>

void QIniFile::load(const QString &file, QIniData *data)
{
	QFile f(file);
	if( f.open(QIODevice::ReadOnly) )
	{
		QByteArray linea;
		QList<QByteArray> bits;
		while( !f.atEnd() )
			if( (linea = f.readLine().replace("\n", "")).count() )
				if( (bits = linea.split('=')).count() == 2 )
					(*data)[QString::fromLatin1(bits[0])] = QString::fromLatin1(bits[1]);
		f.close();
	}
}

void QIniFile::save(const QString &file, const QIniData &data)
{
	QFile f(file);
	if( f.open(QIODevice::WriteOnly) )
	{
		QMap<QString, QString>::const_iterator i;
		for( i = data.constBegin(); i != data.constEnd(); i++ )
			f.write(QString("%1=%2\n").arg(i.key(), i.value()).toLatin1());
		f.close();
	}
}
