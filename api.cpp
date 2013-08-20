#include "api.h"
#include "datacfg.h"
#include "util/json.h"
#include "zlib/zlib.h"
#include "util/s2.h"
#include "influxwnd.h"
#include "deviceinfo.h"

static QByteArray gUncompress(const QByteArray &data);

const QString API::ms_URL="https://m-dot-betaspike.appspot.com/";
API* API::ms_pxInst=NULL;

API::API(QObject *parent) :
    QObject(parent)
{
    ms_pxInst=this;
    m_iNetMgrCycle=0;
    m_iRPCPending=0;
    //m_pxNetMgrRPC=NULL;
    //m_pxCookies=NULL;
}

void API::Init()
{
    if(m_apxNetMgrRPC.count()>0) {return;};
    for(int i=0;i<4;i++)
    {
        QNetworkAccessManager* pxM=new QNetworkAccessManager(this);
        connect(pxM,SIGNAL(finished(QNetworkReply*)),this, SLOT(OnHttpDoneRPC(QNetworkReply*)));
        m_apxNetMgrRPC.append(pxM);
    };

    //if(m_pxNetMgrRPC) {return;};
    //m_pxNetMgrRPC=new QNetworkAccessManager(this);
    //connect(m_pxNetMgrRPC,SIGNAL(finished(QNetworkReply*)),this, SLOT(OnHttpDoneRPC(QNetworkReply*)));

    //m_pxCookies=new NetworkCookieJar(this);
    //m_pxCookies->FromVariant(DataCfg::Get()->m_xUserCfg.m_xData["LoginCookies"]);
    //m_pxCookies->DoEndSession();
    //m_pxNetMgrRPC->setCookieJar(m_pxCookies);

}

void API::Shut()
{
}

QNetworkReply* API::RPCSend(QString p_sName,QVariantMap p_xParams)
{
    QString sURL=API::ms_URL+"rpc/"+p_sName;

    //m_pxCookies->FromVariant(QStringList()<<DataCfg::Get()->m_xUserCfg.m_sAccCookie);

    p_xParams["knobSyncTimestamp"]=QString::number(QDateTime::currentMSecsSinceEpoch());
    //qDebug()<<p_xParams["knobSyncTimestamp"];

    QVariantMap xParams;
    xParams["params"]=p_xParams;

    QByteArray xData=QtJson::Json::serialize(xParams);
    QString sCookie=DataCfg::Get()->m_xUserCfg.m_sAccCookie;
    QString sToken=DataCfg::Get()->m_xPlayer.m_sXsrfToken;
    //qDebug() << sCookie << sToken;

    QNetworkRequest req;
    req.setUrl(QUrl(sURL));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json;charset=UTF-8");
    req.setRawHeader("Cookie",sCookie.toLatin1());
    req.setRawHeader("Accept-Encoding","gzip");
    req.setRawHeader("User-Agent","Nemesis (gzip)");
    req.setRawHeader("X-XsrfToken",sToken.toLatin1());
    req.setRawHeader("Host","m-dot-betaspike.appspot.com");
    req.setRawHeader("Connection","Keep-Alive");

    QNetworkAccessManager* pxM=m_apxNetMgrRPC[m_iNetMgrCycle%m_apxNetMgrRPC.count()];
    m_iNetMgrCycle++;m_iNetMgrCycle%=m_apxNetMgrRPC.count();

    if(true)
    {
        QFile xFile(InfluxWnd::ms_pxInst->m_sPathUser+"_rpc_query.txt");
        if(xFile.open(QIODevice::WriteOnly))
        {
            QList<QByteArray> axHL=req.rawHeaderList();
            foreach(QByteArray a,axHL)
            {
                xFile.write(a);
                xFile.write(":");
                xFile.write(req.rawHeader(a));
                xFile.write("\r\n");
            };
            xFile.write("\r\n");
            xFile.write(xData);
            xFile.close();
        };
    };

    m_iRPCPending++;
    return pxM->post(req,xData);
    //qDebug() << xData;
}

void API::UpdateLastAction()
{
    DataCfg::Get()->m_xUserCfg.m_iLastActionTime=QDateTime::currentMSecsSinceEpoch();
    DataCfg::Get()->m_xUserCfg.m_xLastActionPos=m_xPos;
}

void API::RPCInventory()
{
    QString sCmd="playerUndecorated/getInventory";
    QVariantMap xP;
    QString sLQT=DataCfg::Get()->m_xUserCfg.m_sLastInvSync;
    if(sLQT.trimmed().isEmpty()) {sLQT="0";};
    xP["lastQueryTimestamp"]=sLQT;
    RPCSend(sCmd,xP);
}

void API::RPCHack(QString p_sPortal)
{
    UpdateLastAction();
    QString sCmd="gameplay/collectItemsFromPortal";
    QVariantMap xP;
    xP["knobSyncTimestamp"]="1370000000000";
    xP["playerLocation"]=S2::PosToE6(PosJitter(m_xPos));
    xP["itemGuid"]=p_sPortal;
    RPCSend(sCmd,xP);
}

void API::RPCDrop(QString p_sItem)
{
    UpdateLastAction();
    QString sCmd="gameplay/dropItem";
    QVariantMap xP;
    xP["knobSyncTimestamp"]="1370000000000";
    xP["playerLocation"]=S2::PosToE6(m_xPos);
    xP["itemGuid"]=p_sItem;
    RPCSend(sCmd,xP);
}

void API::RPCPickup(QString p_sItem)
{
    UpdateLastAction();
    QString sCmd="gameplay/pickUp";
    QVariantMap xP;
    xP["knobSyncTimestamp"]="1370000000000";
    xP["playerLocation"]=S2::PosToE6(PosJitter(m_xPos));
    xP["itemGuid"]=p_sItem;
    RPCSend(sCmd,xP);
}

void API::RPCRecycle(QString p_sItem)
{
    QString sCmd="gameplay/recycleItem";
    QVariantMap xP;
    xP["knobSyncTimestamp"]="1370000000000";
    xP["playerLocation"]=S2::PosToE6(PosJitter(m_xPos));
    xP["itemGuid"]=p_sItem;
    RPCSend(sCmd,xP);
}

void API::RPCUseCube(QString p_sItem)
{
    QString sCmd="gameplay/dischargePowerCube";
    QVariantMap xP;
    xP["knobSyncTimestamp"]="1370000000000";
    xP["playerLocation"]=S2::PosToE6(PosJitter(m_xPos));
    xP["itemGuid"]=p_sItem;
    RPCSend(sCmd,xP);
}

void API::RPCFireXMP(QString p_sItem)
{
    UpdateLastAction();
    QString sCmd="gameplay/fireUntargetedRadialWeapon";
    QVariantMap xP;
    xP["knobSyncTimestamp"]="1370000000000";
    xP["playerLocation"]=S2::PosToE6(PosJitter(m_xPos));
    xP["itemGuid"]=p_sItem;
    RPCSend(sCmd,xP);
}

void API::RPCRecharge(QString p_sPortal)
{
    UpdateLastAction();
    QString sCmd="gameplay/rechargeResonatorsV2";
    QVariantMap xP;
    xP["knobSyncTimestamp"]="1370000000000";
    xP["location"]=S2::PosToE6(PosJitter(m_xPos));
    xP["portalGuid"]=p_sPortal;
    xP["portalKeyGuid"]=0;
    xP["resonatorSlots"]=QVariantList()<<0<<1<<2<<3<<4<<5<<6<<7;
    RPCSend(sCmd,xP);
}

void API::RPCFlip(QString p_sPortal,QString p_sItem)
{
    UpdateLastAction();
    QString sCmd="gameplay/flipPortal";
    QVariantMap xP;
    xP["knobSyncTimestamp"]="1370000000000";
    xP["location"]=S2::PosToE6(PosJitter(m_xPos));
    xP["playerLocation"]=xP["location"];
    xP["portalGuid"]=p_sPortal;
    xP["resourceGuid"]=p_sItem;
    RPCSend(sCmd,xP);
}

void API::RPCAddMod(QString p_sPortal,QString p_sItem,int p_iSlot)
{
    UpdateLastAction();
    QString sCmd="gameplay/addMod";
    QVariantMap xP;
    xP["knobSyncTimestamp"]="1370000000000";
    xP["location"]=S2::PosToE6(PosJitter(m_xPos));
    xP["playerLocation"]=xP["location"];
    xP["modResourceGuid"]=p_sItem;
    xP["modableGuid"]=p_sPortal;
    xP["index"]=p_iSlot;
    RPCSend(sCmd,xP);
}

void API::RPCDeploy(QString p_sPortal,QString p_sResonator,int p_iSlot)
{
    UpdateLastAction();
    QString sCmd="gameplay/deployResonatorV2";
    QVariantMap xP;
    xP["knobSyncTimestamp"]="1370000000000";
    xP["location"]=S2::PosToE6(PosJitter(m_xPos));
    xP["portalGuid"]=p_sPortal;
    xP["preferredSlot"]=p_iSlot;
    xP["itemGuids"]=QVariantList()<<p_sResonator;
    RPCSend(sCmd,xP);
}

void API::RPCUpgrade(QString p_sPortal,QString p_sResonator,int p_iSlot)
{
    UpdateLastAction();
    QString sCmd="gameplay/upgradeResonatorV2";
    QVariantMap xP;
    xP["knobSyncTimestamp"]="1370000000000";
    xP["location"]=S2::PosToE6(PosJitter(m_xPos));
    xP["portalGuid"]=p_sPortal;
    xP["resonatorSlotToUpgrade"]=p_iSlot;
    xP["emitterGuid"]=p_sResonator;
    RPCSend(sCmd,xP);
}

void API::RPCLinkQuery(QString p_sPortal,QStringList p_asKeys)
{
    QString sCmd="gameplay/getLinkabilityImpediment";
    QVariantMap xP;
    xP["knobSyncTimestamp"]="1370000000000";
    xP["playerLocation"]=S2::PosToE6(PosJitter(m_xPos));
    xP["originPortalGuid"]=p_sPortal;
    xP["portalLinkKeyGuidSet"]=p_asKeys;
    RPCSend(sCmd,xP);
}

void API::RPCLinkBuild(QString p_sPortalFrom,QString p_sPortalTo,QString p_sKey)
{
    UpdateLastAction();
    QString sCmd="gameplay/createLink";
    QVariantMap xP;
    xP["knobSyncTimestamp"]="1370000000000";
    xP["playerLocation"]=S2::PosToE6(PosJitter(m_xPos));
    xP["originPortalGuid"]=p_sPortalFrom;
    xP["destinationPortalGuid"]=p_sPortalTo;
    xP["linkKeyGuid"]=p_sKey;
    RPCSend(sCmd,xP);
}

QPointF API::PosJitter(QPointF p, float p_fStr)
{
    float r1=((float)qrand()/(float)RAND_MAX)-0.5;
    float r2=((float)qrand()/(float)RAND_MAX)-0.5;
    return QPointF(p.x()+r1*p_fStr*2,p.y()+r1*p_fStr*2);
}

void API::RPCXM()
{
    DataCfg::TEnergy& rxEC=DataCfg::Get()->m_xInfluxData.m_xEnergyCache;
    if(rxEC.m_asGlobs.isEmpty())
    {
        rxEC.m_asGlobs=DataCfg::Get()->m_xEnergy.Fetch(10000);
    };

    int iXM=DataCfg::Get()->m_xPlayer.m_iXMMax-DataCfg::Get()->m_xPlayer.m_iXM;
    qDebug() << "Need" << iXM << "XM";
    //qDebug() << "Fetch:" << m_asXM;
    RPCScan(m_xPos,DataCfg::TEnergy::FetchFrom(rxEC.m_asGlobs,iXM),QString());
}

void API::RPCXMSlurp()
{
    DataCfg::TEnergy& rxEC=DataCfg::Get()->m_xInfluxData.m_xEnergyCache;
    if(rxEC.m_asGlobs.isEmpty())
    {
        rxEC.m_asGlobs=DataCfg::Get()->m_xInfluxData.FetchMultiXM(10000);
    };

    int iXM=DataCfg::Get()->m_xPlayer.m_iXMMax-DataCfg::Get()->m_xPlayer.m_iXM;
    qDebug() << "Need" << iXM << "XM";
    //qDebug() << "Fetch:" << m_asXM;
    RPCScan(m_xPos,DataCfg::TEnergy::FetchFrom(rxEC.m_asGlobs,iXM),QString());

    emit UpdSlurp();
}

void API::RPCScan()
{
    RPCScan(m_xPos,QStringList(),"");
}

void API::RPCScan(QPointF p_xPos, QStringList p_asXM,QString p_sSlurpTarget)
{
    double dRng=0.003;
    double dStp=0.0002;

    QList<quint64> aiCells;
    for(double dX=p_xPos.x()-dRng;dX<p_xPos.x()+dRng;dX+=dStp)
    {
        for(double dY=p_xPos.y()-dRng;dY<p_xPos.y()+dRng;dY+=dStp)
        {
            quint64 iCell=S2::PosToCellID(QPointF(dX,dY));
            iCell=S2::CellIDParent(iCell,16);
            if(aiCells.contains(iCell)) {continue;};
            aiCells.append(iCell);
            //qDebug() << "Cell:" << QString::number(iCell,16) << S2::CellIDLevel(iCell);
        };

    };

    QStringList asCells;
    QVariantList aiDates;
    for(int i=0;i<aiCells.count();i++)
    {
        asCells.append(QString::number(aiCells[i],16));
        aiDates.append(0);
    };

    QString sCmd="gameplay/getObjectsInCells";
    QVariantMap xP;
    xP["playerLocation"]=S2::PosToE6(m_xPos);
    xP["knobSyncTimestamp"]="1370000000000";
    xP["cellsAsHex"]=asCells;
    xP["dates"]=aiDates;
    xP["energyGlobGuids"]=p_asXM;

    QNetworkReply* pxNR=RPCSend(sCmd,xP);
    if(pxNR&&!p_sSlurpTarget.isEmpty())
    {
        pxNR->setObjectName("slurp:"+p_sSlurpTarget);
    };
}

void API::OnHttpDoneRPC(QNetworkReply *p_pxRes)
{
    p_pxRes->deleteLater();
    QString sURL=p_pxRes->url().toString();
    QByteArray xData=p_pxRes->readAll();
    xData=gUncompress(xData);
    qDebug() << "OnHttpDoneRPC::" << sURL;// << xData;
    //qDebug() << "OnHttpDoneRPC::" << sURL << xData;

    m_iRPCPending--;
    emit OnRPCDone(sURL);

    if(true)
    {
        QFile xFile(InfluxWnd::ms_pxInst->m_sPathUser+"_rpc_result.txt");
        if(xFile.open(QIODevice::WriteOnly))
        {
            xFile.write(xData);
            xFile.close();
        };
    };

    QString sErr;
    QVariant xV=QtJson::Json::parse(xData,sErr);
    QVariantMap xM=xV.toMap();
    QString sTimeStamp=xM["result"].toString();
    QVariantMap xGB=xM["gameBasket"].toMap();
    QVariantList xGE=xGB["gameEntities"].toList();
    QVariantList xInv=xGB["inventory"].toList();
    QVariantList xEng=xGB["energyGlobGuids"].toList();
    QStringList xDEG=xGB["deletedEntityGuids"].toStringList();

    QString sRPCName=p_pxRes->objectName();
    if(sRPCName.startsWith("slurp:"))
    {
        QString sName=sRPCName.mid(6);
        DataCfg::TInfluxData::TPlaceXM* pxP=DataCfg::Get()->m_xInfluxData.GetPlace(sName);
        if(!pxP)
        {
            DataCfg::Get()->m_xInfluxData.m_axPlaces.append(DataCfg::TInfluxData::TPlaceXM());
            pxP=&DataCfg::Get()->m_xInfluxData.m_axPlaces.last();
            pxP->m_sName=sName;
        };
        pxP->m_xEnergy.Load(xEng);
        emit UpdSlurp();
        return;
    };

    if(xDEG.count()>0)
    {
        qDebug() << "deletedEntityGuids:" << xDEG.count();
        DataCfg::Get()->m_xGameEnts.DeleteEntities(xDEG);
        if(DataCfg::Get()->m_xInventory.DeleteEntities(xDEG))
        {
            emit UpdInventory();
        };
    };

    if(sURL.contains("gameplay/collectItemsFromPortal")||
       sURL.contains("gameplay/pickUp"))
    {
        QStringList asNewItems=xM["result"].toMap()["addedGuids"].toStringList();
        qDebug() << "Items Got:" << asNewItems.count();
        if(DataCfg::Get()->m_xInventory.Append(xInv))
        {
            emit UpdInventory(asNewItems);
        };
    };
    if(xM.contains("error"))
    {
        qDebug() << "RPCError:" << xM["error"].toString();
    };
    if(xGB.contains("playerEntity"))
    {
        QVariantList xPE=xGB["playerEntity"].toList();
        DataCfg::Get()->m_xPlayer.Update(xPE);
        emit UpdPlayer();
    };

    if(sURL.contains("playerUndecorated/getInventory"))
    {
        DataCfg::Get()->m_xInventory.Append(xInv);
        DataCfg::Get()->m_xUserCfg.m_sLastInvSync=sTimeStamp;
        emit UpdInventory();
        return;
    };
    if(sURL.contains("gameplay/deployResonatorV2")||
       sURL.contains("gameplay/upgradeResonatorV2")||
       sURL.contains("gameplay/rechargeResonatorsV2")||
       sURL.contains("gameplay/addMod")||
       sURL.contains("gameplay/fireUntargetedRadialWeapon")||
       sURL.contains("gameplay/flipPortal"))
    {
        if(DataCfg::Get()->m_xGameEnts.Update(xGE))
        {
            emit UpdPortals();
            return;
        };
    };
    if(sURL.contains("gameplay/getObjectsInCells"))
    {
        DataCfg::Get()->m_xGameEnts.Load(xGE);
        DataCfg::Get()->m_xEnergy.Load(xEng);
        emit UpdPortals();
        return;
    };
}

//////////////////////////////////////////////////////////////////////

static QByteArray gUncompress(const QByteArray &data)
{
    if (data.size() <= 4) {
        qWarning() << "gUncompress: Input data is truncated";
        return QByteArray();
    }

    QByteArray result;

    int ret;
    z_stream strm;
    static const int CHUNK_SIZE = 1024;
    char out[CHUNK_SIZE];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = data.size();
    strm.next_in = (Bytef*)(data.data());

    ret = inflateInit2(&strm, 15 +  32); // gzip decoding
    if (ret != Z_OK)
        return QByteArray();

    // run inflate()
    do {
        strm.avail_out = CHUNK_SIZE;
        strm.next_out = (Bytef*)(out);

        ret = inflate(&strm, Z_NO_FLUSH);
        Q_ASSERT(ret != Z_STREAM_ERROR);  // state not clobbered

        switch (ret) {
        case Z_NEED_DICT:
            ret = Z_DATA_ERROR;     // and fall through
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            (void)inflateEnd(&strm);
            return QByteArray();
        }

        result.append(out, CHUNK_SIZE - strm.avail_out);
    } while (strm.avail_out == 0);

    // clean up and return
    inflateEnd(&strm);
    return result;
}

