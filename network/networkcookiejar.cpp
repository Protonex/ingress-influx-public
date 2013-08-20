#include "networkcookiejar.h"
#include "qvariant.h"

NetworkCookieJar::NetworkCookieJar(QObject *parent) : QNetworkCookieJar(parent)
{

}

QList<QNetworkCookie> NetworkCookieJar::cookiesForUrl(const QUrl &p_xUrl) const
{
	return QNetworkCookieJar::cookiesForUrl(p_xUrl);
}

bool NetworkCookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &p_axCookies, const QUrl &p_xUrl)
{
	bool bRes=QNetworkCookieJar::setCookiesFromUrl(p_axCookies,p_xUrl);
	/*forint(i,p_axCookies)
	{
		const QNetworkCookie& xC=p_axCookies[i];
		//qDebug() << "SCookie:" << xC;
	};*/
	return bRes;
}

void NetworkCookieJar::DoEndSession()
{
	QDateTime xNow=QDateTime::currentDateTime().toTimeSpec(Qt::UTC);
	QList<QNetworkCookie> axC=allCookies();
	forint(i,axC)
	{
		const QNetworkCookie& xC=axC[i];
		if(xC.isSessionCookie()) {axC.removeAt(i);i--;continue;};
		if(xNow>xC.expirationDate()) {axC.removeAt(i);i--;continue;};
	};
	setAllCookies(axC);
}

QVariant NetworkCookieJar::ToVariant() const
{
	QVariantList axRet;
	QList<QNetworkCookie> axC=allCookies();
	forint(i,axC)
	{
		const QNetworkCookie& xC=axC[i];
		//qDebug() << "WCookie:" << xC;
		axRet.append(xC.toRawForm());
	};
	return axRet;
}

void NetworkCookieJar::FromVariant(const QVariant& p_xV)
{
	QList<QNetworkCookie> axC;
	QVariantList axRet=p_xV.toList();
	forint(i,axRet)
	{
		const QVariant& xC=axRet[i];
		QList<QNetworkCookie> axCParsed=QNetworkCookie::parseCookies(xC.toString().toUtf8());
		foreach(const QNetworkCookie& xC,axCParsed)
		{
			//qDebug() << "RCookie:" << xC;
			axC.append(xC);
		};
	};
	setAllCookies(axC);
}

