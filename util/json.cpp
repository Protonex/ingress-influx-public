/* Copyright 2011 Eeli Reilin. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL EELI REILIN OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of Eeli Reilin.
 */

/**
 * \file json.cpp
 */

#include "json.h"
#include <iostream>
#include <QtCore/qrect.h>

#ifdef POWER_GRID
#include "pal.h"
#include "simplecrypt.h"

int CryptString::qt_metatype_id() {return QMetaTypeId<CryptString>::qt_metatype_id();}

class QVariantCryptStringCvt : public QVariant
{
public:
	static void Init();
};

static QVariant::f_convert g_pxCvtOrg=NULL;
static bool ConvertCryptString(const QVariant::Private *d, QVariant::Type t, void *result, bool *ok)
{
	if(t==QVariant::String&&d->type==CryptString::qt_metatype_id())
	{
		const void* pxD=d->is_shared ? d->data.shared->ptr : reinterpret_cast<const void *>(&d->data.ptr);
		CryptString* pxCS=(CryptString*)pxD;
		QString *str = static_cast<QString *>(result);
		*str=pxCS->m_sValue;
		return true;
	};
	return g_pxCvtOrg(d,t,result,ok);
}

void QVariantCryptStringCvt::Init()
{
	if(g_pxCvtOrg==NULL)
	{
		static const QVariant::Handler vh={
				handler->construct,
				handler->clear,
				handler->isNull,
		#ifndef QT_NO_DATASTREAM
				handler->load,
				handler->save,
		#endif
				handler->compare,
				ConvertCryptString,
				handler->canConvert,
		#if !defined(QT_NO_DEBUG_STREAM) && !defined(Q_BROKEN_DEBUG_STREAM)
				handler->debugStream
		#else
				0
		#endif
		};

		g_pxCvtOrg=handler->convert;
		handler=&vh;

		int iDbg=0;
	};
}
#endif

namespace QtJson
{

static QString sanitizeString(QString str)
{
	str.replace(QLatin1String("\\"), QLatin1String("\\\\"));
	str.replace(QLatin1String("\""), QLatin1String("\\\""));
	str.replace(QLatin1String("\b"), QLatin1String("\\b"));
	str.replace(QLatin1String("\f"), QLatin1String("\\f"));
	str.replace(QLatin1String("\n"), QLatin1String("\\n"));
	str.replace(QLatin1String("\r"), QLatin1String("\\r"));
	str.replace(QLatin1String("\t"), QLatin1String("\\t"));
	return QString(QLatin1String("\"%1\"")).arg(str);
}

static QByteArray join(const QList<QByteArray> &list, const QByteArray &sep)
{
	QByteArray res;
	Q_FOREACH(const QByteArray &i, list)
	{
		if(!res.isEmpty())
		{
			res += sep;
		}
		res += i;
	}
	return res;
}

/**
 * parse
 */
QVariant Json::parse(const QString &json)
{
	QString error;
	return Json::parse(json, error);
}

#ifdef POWER_GRID
static void DoCryptString(QVariant& p_xV,QString p_sCtx,bool p_bDecrypt)
{
	QVariant::Type eType=p_xV.type();
	if(eType==QVariant::Map)
	{
		QVariantMap xM=p_xV.toMap();
		foreach(QString s,xM.keys())
		{
			DoCryptString(xM[s],p_sCtx+"/"+s,p_bDecrypt);
		};
		p_xV=xM;
	}
	else if(eType==QVariant::List)
	{
		QVariantList xL=p_xV.toList();
		forint(i,xL)
		{
			DoCryptString(xL[i],p_sCtx,p_bDecrypt);
		};
		p_xV=xL;
	}
	else if(eType==QVariant::UserType&&p_xV.userType()==CryptString::qt_metatype_id())
	{
		QString sVal=p_xV.value<CryptString>();
		static QByteArray sCID=PAL::GetComputerID().toUtf8();
		SimpleCrypt xC(QByteArray::fromBase64(HMAC_SHA1(sCID,"CryptString"+p_sCtx).toUtf8()));
		if(p_bDecrypt)
		{
			sVal=xC.DecryptToString(sVal);
		}
		else
		{
			sVal=xC.EncryptToString(sVal);
		};
		p_xV=CryptString(sVal);
	};
}
#endif


/**
 * parse
 */
QVariant Json::parse(const QString &json, QString &error)
{
#ifdef POWER_GRID
	QVariantCryptStringCvt::Init();
#endif
	error="";

	//Return an empty QVariant if the JSON data is either null or empty
	if(!json.isNull() || !json.isEmpty())
	{
		QString data = json;
		//We'll start from index 0
		int index = 0;

		//Parse the first value
		QVariant value = Json::parseValue(data, index, error);

#ifdef POWER_GRID
		DoCryptString(value,"",true);
#endif

		//Return the parsed value
		return value;
	}
	else
	{
		//Return the empty QVariant
		return QVariant();
	}
}

void Json::SetError(const QString &json,int index,QString &error,QString sMsg)
{
	int iL=1;
	int iC=1;
	foreach(QChar c,json)
	{
		if(c=='\n') {iL++;iC=1;} else {iC++;};
		index--;
		if(index<=0) {break;};
	};

	error=QString("(%1:%2) %3").arg(iL).arg(iC).arg(sMsg);
}

QByteArray Json::serialize(const QVariant &data)
{
	QString error;
	return Json::serialize(data, error);
}

QByteArray Json::serialize(const QVariant &d,QString &error,int p_iIndent)
{
	QVariant data=d;data.detach();
#ifdef POWER_GRID
	DoCryptString(data,"",false);
#endif
	return serialize_int(data,error,p_iIndent);
}

QByteArray Json::serialize_int(const QVariant &data,QString &error,int p_iIndent)
{
	QByteArray str;
	error="";
	const QByteArray xTab("\t");
	if(p_iIndent>0) {str+=xTab.repeated(p_iIndent);};
	if(p_iIndent!=NO_INDENT&&p_iIndent<0) {p_iIndent=-p_iIndent;};
	QVariant::Type eType=data.type();
	if(!data.isValid()) // invalid or null?
	{
		str = "null";
	}
	else if((eType == QVariant::List) || (eType == QVariant::StringList)) // variant is a list?
	{
		bool bStringList=true; //eType may be inconsistent

		QList<QByteArray> values;
		const QVariantList list = data.toList();
		Q_FOREACH(const QVariant& v, list)
		{
			if(v.type()!=QVariant::String) {bStringList=false;};
			QByteArray serializedValue = serialize_int(v,error,p_iIndent>=0?(p_iIndent+1):NO_INDENT);
			if(serializedValue.isNull())
			{
				error="serialize.list.1:"+error;
				break;
			}
			values << serializedValue;
		}

		if(p_iIndent>=0)
		{
			QByteArray sTR=xTab.repeated(p_iIndent);
			if(bStringList)
			{
				QByteArray sValsShort=join(values,",");
				if(sValsShort.length()<=80)
				{
					str = "["+sValsShort+"]";
				}
				else
				{
					str = "[\n" + sTR+xTab + join(values,",\n"+sTR+xTab) + "\n"+sTR+"]";
				};
			}
			else
			{
				str = "[\n" + join(values,",\n") + "\n"+sTR+"]";
			};
		}
		else
		{
			str = "[" + join(values,",") + "]";
		};
	}
	else if(eType == QVariant::Map) // variant is a map?
	{
		const QVariantMap vmap = data.toMap();
		QMapIterator<QString, QVariant> it( vmap );
		QList<QByteArray> pairs;
		while(it.hasNext())
		{
			it.next();
			QByteArray serializedValue = serialize_int(it.value(),error,p_iIndent>=0?(-(p_iIndent+1)):NO_INDENT);
			if(serializedValue.isNull())
			{
				error="serialize.map.1:"+error;
				break;
			}
			if(p_iIndent>=0)
			{
				QString sKey=it.key();
				bool bAlphaNum=true;
                foreach(QChar c,sKey) {if(!IsAlphaNum(c.toLatin1())) {bAlphaNum=false;break;};};
				QByteArray xKey=bAlphaNum?sKey.toUtf8():sanitizeString(sKey).toUtf8();
				pairs << xTab.repeated(p_iIndent+1) + xKey + ":" + serializedValue;
			}
			else
			{
				pairs << sanitizeString(it.key()).toUtf8() + ":" + serializedValue;
			};
		}
		if(p_iIndent>=0)
		{
			str += "{\n" + join(pairs,",\n") + "\n" + xTab.repeated(p_iIndent) + "}";
		}
		else
		{
			str += "{" + join(pairs,",") + "}";
		};
	}
	else if((eType == QVariant::String) || (eType == QVariant::ByteArray)) // a string or a byte array?
	{
		str = sanitizeString(data.toString()).toUtf8();
	}
#ifdef POWER_GRID
	else if(eType==QVariant::UserType&&data.userType()==CryptString::qt_metatype_id())
	{
		str = ("!"+sanitizeString(data.toString())).toUtf8();
	}
#endif
	else if(eType == QVariant::Double) // double?
	{
		str = QByteArray::number(data.toDouble(), 'g', 20);
		//a Number is a Number (javascript doesn't know integer
		/*if(!str.contains(".") && ! str.contains("e"))
		{
			str += ".0";
		}*/
	}
	else if (eType == QVariant::Bool) // boolean value?
	{
		str = data.toBool() ? "true" : "false";
	}
	else if (eType == QVariant::Rect) // rect
	{
		QRect val=data.toRect();
		str = QString("[%1,%2,%3,%4]").arg(val.x()).arg(val.y()).arg(val.width()).arg(val.height()).toUtf8();
	}
	else if (eType == QVariant::RectF) // rect
	{
		QRectF val=data.toRect();
		str = QString("[%1,%2,%3,%4]").arg(val.x()).arg(val.y()).arg(val.width()).arg(val.height()).toUtf8();
	}
	else if (eType == QVariant::ULongLong) // large unsigned number?
	{
		str = QByteArray::number(data.value<qulonglong>());
	}
	else if ( data.canConvert<qlonglong>() ) // any signed number?
	{
		str = QByteArray::number(data.value<qlonglong>());
	}
	else if (data.canConvert<long>())
	{
		str = QString::number(data.value<long>()).toUtf8();
	}
	else if (data.canConvert<QString>()) // can value be converted to string?
	{
		// this will catch QDate, QDateTime, QUrl, ...
		str = sanitizeString(data.toString()).toUtf8();
	}
	else
	{
		error="serialize.unknowntype";
	}
	if (error.length()<=0)
	{
		return str;
	}
	else
	{
		return QByteArray();
	}
}

/**
 * parseValue
 */
QVariant Json::parseValue(const QString &json, int &index, QString &error)
{
	//Determine what kind of data we should parse by
	//checking out the upcoming token
	QChar c;
	switch(Json::lookAhead(json, index, c))
	{
		case JsonTokenString:
			return Json::parseString(json, index,false,false, error);
#ifdef POWER_GRID
		case JsonTokenStringCrypt:
			return CryptString(Json::parseString(json, index,false,true, error).toString());
#endif
		case JsonTokenStringVerbatim:
			return Json::parseString(json, index,false,true, error);
		case JsonTokenNumber:
			return Json::parseNumber(json, index);
		case JsonTokenCurlyOpen:
			return Json::parseObject(json, index, error);
		case JsonTokenSquaredOpen:
			return Json::parseArray(json, index, error);
		case JsonTokenTrue:
			Json::nextToken(json, index, c);
			return QVariant(true);
		case JsonTokenFalse:
			Json::nextToken(json, index, c);
			return QVariant(false);
		case JsonTokenNull:
			Json::nextToken(json, index, c);
			return QVariant();
		case JsonTokenNone:
			break;
	}

	//If there were no tokens, flag the failure and return an empty QVariant
	SetError(json,index,error,"parsevalue.notokens:"+QString(c));
	return QVariant();
}

/**
 * parseObject
 */
QVariant Json::parseObject(const QString &json, int &index, QString &error)
{
	QVariantMap map;
	QChar c;
	int token;

	//Get rid of the whitespace and increment index
	Json::nextToken(json, index, c);

	//Loop through all of the key/value pairs of the object
	bool done = false;
	while(!done)
	{
		//Get the upcoming token
		token = Json::lookAhead(json, index, c);

		if(token == JsonTokenComma)
		{
			Json::nextToken(json, index, c);
		}
		else if(token == JsonTokenCurlyClose)
		{
			Json::nextToken(json, index, c);
			return map;
		}
        else if(token==JsonTokenString||token==JsonTokenNone||token==JsonTokenNumber)
		{
            bool bUnquoted=token==JsonTokenNone||token==JsonTokenNumber;
			if(bUnquoted)
			{
                if(!IsAlphaNum(c.toLatin1()))
				{
					SetError(json,index,error,"parseobject.invtoken:" + QString(c));
					return QVariantMap();
				};
			};

			//Parse the key/value pair's name
			QString name = Json::parseString(json, index,bUnquoted,false, error).toString();

			if(error.length()>0)
			{
				return QVariantMap();
			}

			//Get the next token
			token = Json::nextToken(json, index, c);

			//If the next token is not a colon, flag the failure
			//return an empty QVariant
			if(token != JsonTokenColon)
			{
				SetError(json,index,error,"parseobject.expected.colon");
				return QVariant(QVariantMap());
			}

			//Parse the key/value pair's value
			QVariant value = Json::parseValue(json, index, error);

			if(error.length()>0)
			{
				return QVariantMap();
			}

			//Assign the value to the key in the map
			map[name] = value;
		}
		else
		{
			SetError(json,index,error,"parseobject.invtoken:" + QString(c));
			return QVariantMap();
		};
	}

	//Return the map successfully
	return QVariant(map);
}

/**
 * parseArray
 */
QVariant Json::parseArray(const QString &json, int &index, QString &error)
{
	QVariantList xArray;
	QChar c;
	Json::nextToken(json, index, c);

	bool done = false;
	while(!done)
	{
		int token = Json::lookAhead(json, index, c);

		if(token == JsonTokenNone)
		{
			SetError(json,index,error,"parseerror.invtoken:"+QString(c));
			return QVariantList();
		}
		else if(token == JsonTokenComma)
		{
			Json::nextToken(json, index, c);
		}
		else if(token == JsonTokenSquaredClose)
		{
			Json::nextToken(json, index, c);
			break;
		}
		else
		{
			QVariant value = Json::parseValue(json, index, error);

			if(error.length()>0)
			{
				return QVariantList();
			}

			xArray.append(value);
		}
	}

	return QVariant(xArray);
}

/**
 * parseString
 */
QVariant Json::parseString(const QString &json, int &index, bool unquoted,bool verbatim, QString &error)
{
	QString s;
	QChar c;

	Json::eatWhitespace(json, index);

	if(!unquoted)
	{
		c = json[index++];
	};
	if(verbatim)
	{
		c = json[index++];
	};

	bool complete = false;
	while(!complete)
	{
		if(index == json.size())
		{
			break;
		}

		c = json[index++];

		if(unquoted)
		{
            if(!IsAlphaNum(c.toLatin1()))
			{
				index--;
				complete=true;
				break;
			};
		};

		if(c == '\"')
		{
			complete = true;
			break;
		}
		else if(c == '\\'&&!verbatim)
		{
			if(index == json.size())
			{
				break;
			}

			c = json[index++];

			if(c == '\"')
			{
				s.append('\"');
			}
			else if(c == '\\')
			{
				s.append('\\');
			}
			else if(c == '/')
			{
				s.append('/');
			}
			else if(c == 'b')
			{
				s.append('\b');
			}
			else if(c == 'f')
			{
				s.append('\f');
			}
			else if(c == 'n')
			{
				s.append('\n');
			}
			else if(c == 'r')
			{
				s.append('\r');
			}
			else if(c == 't')
			{
				s.append('\t');
			}
			else if(c == 'u')
			{
				int remainingLength = json.size() - index;

				if(remainingLength >= 4)
				{
					QString unicodeStr = json.mid(index, 4);

					int symbol = unicodeStr.toInt(0, 16);

					s.append(QChar(symbol));

					index += 4;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			s.append(c);
		}
	}

	if(!complete)
	{
		SetError(json,index,error,"parsestring.incomplete:"+s);
		return QVariant();
	}

	return QVariant(s);
}

/**
 * parseNumber
 */
QVariant Json::parseNumber(const QString &json, int &index)
{
	Json::eatWhitespace(json, index);

	int lastIndex = Json::lastIndexOfNumber(json, index);
	int charLength = (lastIndex - index) + 1;
	QString numberStr;

	numberStr = json.mid(index, charLength);

	index = lastIndex + 1;

	if (numberStr.contains('.')) {
		return QVariant(numberStr.toDouble(NULL));
	} else if (numberStr.startsWith('-')) {
		return QVariant(numberStr.toLongLong(NULL));
	} else {
		return QVariant(numberStr.toULongLong(NULL));
	}
}

/**
 * lastIndexOfNumber
 */
int Json::lastIndexOfNumber(const QString &json, int index)
{
	int lastIndex;

	for(lastIndex = index; lastIndex < json.size(); lastIndex++)
	{
		if(QString("0123456789+-.eE").indexOf(json[lastIndex]) == -1)
		{
			break;
		}
	}

	return lastIndex -1;
}

/**
 * eatWhitespace
 */
void Json::eatWhitespace(const QString &json, int &index)
{
	bool bInLineComment=false;
	bool bMultiLineComment=false;
	for(; index < json.size(); index++)
	{
        char c=json[index].toLatin1();
		bool bIsWhitespace=IsWhitespace(c);
		bool bIsNewline=IsNewline(c);
		if(bIsNewline&&bInLineComment) {bInLineComment=false;};
		if(bIsWhitespace) {continue;};
        if(c=='/'&&json[index+1].toLatin1()==c&&!bMultiLineComment) {bInLineComment=true;index++;continue;};
        if(c=='/'&&json[index+1].toLatin1()=='*'&&!bInLineComment) {bMultiLineComment=true;index++;continue;};
        if(c=='*'&&json[index+1].toLatin1()=='/'&&bMultiLineComment) {bMultiLineComment=false;bInLineComment=false;index++;continue;};
		if(bInLineComment||bMultiLineComment)
		{
			continue;
		};
		break;
	};
}

/**
 * lookAhead
 */
int Json::lookAhead(const QString &json, int index, QChar& c)
{
	int saveIndex = index;
	return Json::nextToken(json, saveIndex, c);
}

/**
 * nextToken
 */
int Json::nextToken(const QString &json, int &index, QChar &c)
{
	Json::eatWhitespace(json, index);

	if(index == json.size())
	{
		return JsonTokenNone;
	}

	c = json[index];
	index++;
    switch(c.toLatin1())
	{
		case '{': return JsonTokenCurlyOpen;
		case '}': return JsonTokenCurlyClose;
		case '[': return JsonTokenSquaredOpen;
		case ']': return JsonTokenSquaredClose;
		case ',': return JsonTokenComma;
		case '"': return JsonTokenString;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		case '-': return JsonTokenNumber;
		case ':': return JsonTokenColon;
	}

	index--;

	int remainingLength = json.size() - index;

	//@"
	if(remainingLength >= 2)
	{
		if (json[index] == '@' && json[index + 1] == '"')
		{
			index += 2;
			return JsonTokenStringVerbatim;
		}
		if (json[index] == '!' && json[index + 1] == '"')
		{
			index += 2;
			return JsonTokenStringCrypt;
		}
	}

	//True
	if(remainingLength >= 4)
	{
		if (json[index] == 't' && json[index + 1] == 'r' &&
			json[index + 2] == 'u' && json[index + 3] == 'e')
		{
			index += 4;
			return JsonTokenTrue;
		}
	}

	//False
	if (remainingLength >= 5)
	{
		if (json[index] == 'f' && json[index + 1] == 'a' &&
			json[index + 2] == 'l' && json[index + 3] == 's' &&
			json[index + 4] == 'e')
		{
			index += 5;
			return JsonTokenFalse;
		}
	}

	//Null
	if (remainingLength >= 4)
	{
		if (json[index] == 'n' && json[index + 1] == 'u' &&
			json[index + 2] == 'l' && json[index + 3] == 'l')
		{
			index += 4;
			return JsonTokenNull;
		}
	}

	return JsonTokenNone;
}


} //end namespace
