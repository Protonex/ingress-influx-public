#include "dlglogin.h"
#include "api.h"
#include "util/json.h"
#include "datacfg.h"
#include "pch.h"
#include "QtWebKitWidgets/QWebFrame"
#include "deviceinfo.h"
#include "decompiler/decompasmx86.h"

#define VersionNemisis "2013-07-12T15:48:09Z d6f04b1fab4f opt" //1.31.1
#define VersionDevice "4.1.1" //Android
#define VersionIngress "v1.31.1"

DlgLogin::DlgLogin(QWidget *parent) :
    QWebView(parent)
{
    setWindowTitle(tr("Ingress Login"));
    resize(1024,600);

    QVariantMap xM;
    xM["nemesisSoftwareVersion"]=VersionNemisis;
    xM["deviceSoftwareVersion"]=VersionDevice;

    //DecompAsmX86 xDC;xDC.Do();return;

    DeviceInfo xD;
    /*xM["a"]=*/xD.ToHandshakeEncoded();
    //return;

    QString sJ=QString::fromUtf8(QtJson::Json::serialize(xM).toPercentEncoding());
    QString sURL=API::ms_URL+"handshake?json="+sJ;

    m_pxNetMgrWWW=new QNetworkAccessManager(this);
    connect(m_pxNetMgrWWW,SIGNAL(finished(QNetworkReply*)),this, SLOT(OnHttpDoneWWW(QNetworkReply*)));

    m_pxCookies=new NetworkCookieJar(this);
    m_pxCookies->FromVariant(DataCfg::Get()->m_xUserCfg.GetLoginCookies());
    m_pxCookies->DoEndSession();

    m_pxNetMgrWWW->setCookieJar(m_pxCookies);

    connect(this,SIGNAL(urlChanged(QUrl)),this,SLOT(OnURLChanged(QUrl)));
    connect(this,SIGNAL(loadFinished(bool)),this,SLOT(OnLoadFinished(bool)));

    qDebug() << "Login:" << sURL;
    page()->setNetworkAccessManager(m_pxNetMgrWWW);
    load(QUrl(sURL));
}

DlgLogin::~DlgLogin()
{
    DataCfg::Get()->SaveUserCfg();
}

void DlgLogin::OnHttpDoneWWW(QNetworkReply *p_pxRes)
{
    QString sURL=p_pxRes->url().toString();
    QByteArray xData=p_pxRes->readAll();
    //qDebug() << "OnHttpDoneWWW::" << sURL << xData;

    //m_pxCookies->DoEndSession();

    QVariant xVCookies=m_pxCookies->ToVariant();
    DataCfg::Get()->m_xUserCfg.SetLoginCookies(xVCookies);
    //InfluxWnd::ms_pxInst->SaveUserCfg();
    //qDebug() << "Kekse:" << xVCookies;*/
}

void DlgLogin::OnURLChanged(const QUrl& p_xURL)
{
    m_sURL=p_xURL.toString();
    bool bDone=m_sURL.contains("handshake?json");
    if(!bDone) {show();};
    //qDebug() << "OnURLChanged::" << m_sURL;
}

void DlgLogin::OnLoadFinished(bool p_bOk)
{
    bool bDone=m_sURL.contains("handshake?json");
    qDebug() << "OnLoadFinished::" << p_bOk << bDone << m_sURL;
    if(!bDone) {return;};

    QString sSrc=page()->mainFrame()->toPlainText();
    if(sSrc.startsWith("while(1);")) {sSrc=sSrc.mid(9);};

    QString sErr;
    QVariant xV=QtJson::Json::parse(sSrc,sErr);
    QVariantMap xM=xV.toMap();

    DataCfg::Get()->m_xPlayer.Load(xM["result"].toMap());
    emit LoginOk();

    qDebug() << sSrc;
    close();
}
