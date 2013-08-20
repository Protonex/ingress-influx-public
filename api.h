#ifndef API_H
#define API_H

#include <QtCore>
#include "network/networkcookiejar.h"
#include <QNetworkAccessManager>

class API : public QObject
{
    Q_OBJECT
public:
    explicit API(QObject *parent = 0);

    static const QString ms_URL;

    //QNetworkAccessManager* m_pxNetMgrRPC;
    QList<QNetworkAccessManager*> m_apxNetMgrRPC;
    int m_iNetMgrCycle;

    int m_iRPCPending;
    QPointF m_xPos;

    void Init();
    void Shut();
    static API* Get() {return ms_pxInst;}
    void UpdateLastAction();

    void RPCInventory();
    void RPCScan();
    void RPCScan(QPointF p_xPos,QStringList p_asXM,QString p_sSlurpTarget);
    void RPCXM();
    void RPCXMSlurp();
    void RPCHack(QString p_sPortal);
    void RPCRecharge(QString p_sPortal);
    void RPCDrop(QString p_sItem);
    void RPCPickup(QString p_sItem);
    void RPCRecycle(QString p_sItem);
    void RPCUseCube(QString p_sItem);
    void RPCFireXMP(QString p_sItem);
    void RPCFlip(QString p_sPortal, QString p_sItem);
    void RPCAddMod(QString p_sPortal, QString p_sItem,int p_iSlot);
    void RPCDeploy(QString p_sPortal,QString p_sResonator,int p_iSlot);
    void RPCUpgrade(QString p_sPortal, QString p_sResonator, int p_iSlot);
    void RPCLinkQuery(QString p_sPortal,QStringList p_asKeys);
    void RPCLinkBuild(QString p_sPortalFrom,QString p_sPortalTo,QString p_sKey);

    QNetworkReply *RPCSend(QString p_sName, QVariantMap p_xParams);
    static QPointF PosJitter(QPointF p,float p_fStr=0.000015f);
signals:
    void UpdInventory(QStringList p_asNew=QStringList());
    void UpdPortals();
    void UpdPlayer();
    void UpdSlurp();
    void OnRPCDone(QString p_sURL);
    
public slots:
    void OnHttpDoneRPC(QNetworkReply *p_pxRes);

private:
    static API* ms_pxInst;
};

#endif // API_H
