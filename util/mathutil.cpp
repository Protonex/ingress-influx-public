#include "../pch.h"
#include <QtCore>
#include "mathutil.h"

QStringList SplitString(const QString& s,const QString &sep, QString::SplitBehavior behavior, Qt::CaseSensitivity cs)
{
	QStringList l=s.split(sep,behavior,cs);
	forint(i,l)
	{
		QString& s=l[i];
		bool bEsc=s.endsWith('\\')&&!s.endsWith("\\\\");
		if(bEsc) {s=s.left(s.length()-1);};
		if(!bEsc||i>=l.size()-1) {continue;};
		s+=sep;
		s+=l[i+1];
		l.removeAt(i+1);
	};
	return l;
}

static inline bool caseInsensitiveLessThan(const QString &s1, const QString &s2)
{
   return s1.localeAwareCompare(s2) < 0;
}

void StringListSortCaseInsensitive(QStringList& p_asVals)
{
	qSort(p_asVals.begin(), p_asVals.end(), caseInsensitiveLessThan);
}

QByteArray ReadFile(QString p_sFileName)
{
	QFile xF(p_sFileName);
	if(!xF.open(QFile::ReadOnly)) {return QByteArray();};
	return xF.readAll();
}

