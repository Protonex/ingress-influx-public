#ifndef NETWORKCOOKIEJAR_H
#define NETWORKCOOKIEJAR_H

#include "../pch.h"
#include <qnetworkcookie.h>

class NetworkCookieJar : public QNetworkCookieJar
{
	Q_OBJECT
public:
	NetworkCookieJar(QObject *parent = 0);

	QList<QNetworkCookie> cookiesForUrl(const QUrl &p_xUrl) const;
	bool setCookiesFromUrl(const QList<QNetworkCookie> &p_axCookies, const QUrl &p_xUrl);

	QVariant ToVariant() const;
	void FromVariant(const QVariant& p_xV);

	QList<QNetworkCookie> allCookies() const {return QNetworkCookieJar::allCookies();}

public slots:
	void DoEndSession();
};

#endif

