#ifndef DLGLOGIN_H
#define DLGLOGIN_H

#include <QtCore>
#include <QtWebKitWidgets/QWebView>
#include "network/networkcookiejar.h"

class DlgLogin : public QWebView
{
    Q_OBJECT
public:
    explicit DlgLogin(QWidget *parent = 0);
    ~DlgLogin();

    QNetworkAccessManager* m_pxNetMgrWWW;
    NetworkCookieJar* m_pxCookies;
    QString m_sURL;
    
signals:
    void LoginOk();
public slots:
    void OnHttpDoneWWW(QNetworkReply *p_pxRes);
    void OnURLChanged(const QUrl& p_xURL);
    void OnLoadFinished(bool p_bOk);
};

#endif // DLGLOGIN_H
