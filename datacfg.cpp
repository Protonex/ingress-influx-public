#include "datacfg.h"
#include "influxwnd.h"
#include "util/json.h"
#include "util/s2.h"

DataCfg* DataCfg::ms_pxInst=NULL;

DataCfg::DataCfg()
{
    ms_pxInst=this;
}

QVariantMap DataCfg::MergeVarMaps(QVariantMap p_xA,QVariantMap p_xB,QStringList p_asIgnore)
{
    QVariantMap xRes=p_xA;

    foreach(QString sB,p_xB.keys())
    {
        if(p_asIgnore.indexOf(sB)!=-1) {continue;};
        QVariant vB=p_xB[sB];
        if(vB.type()!=QVariant::Map)
        {
            xRes[sB]=vB;
        }
        else
        {
            xRes[sB]=MergeVarMaps(xRes[sB].toMap(),vB.toMap(),p_asIgnore);
        };
    };

    return xRes;
}

void DataCfg::TObj::Load(const QVariant& p_xVM)
{
    m_xData=p_xVM;
}

QVariant DataCfg::TObj::Export()
{
    return m_xData;
}

void DataCfg::TPlayer::Load(const QVariant& p_xVM)
{
    TObj::Load(p_xVM);
    QVariantMap xM=m_xData.toMap();
    m_bCanPlay=xM["canPlay"].toBool();
    m_sNick=xM["nickname"].toString();
    m_sXsrfToken=xM["xsrfToken"].toString();

    QVariantList axPE=xM["playerEntity"].toList();
    if(axPE.size()<3) {return;};
    m_sGUID=axPE[0].toString();
    QVariantMap xPE=axPE[2].toMap();

    QVariantMap xCT=xPE["controllingTeam"].toMap();
    m_eTeam=(xCT["team"].toString()=="ALIENS")?T_Aliens:T_Humans;

    QVariantMap xPP=xPE["playerPersonal"].toMap();

    m_iAP=xPP["ap"].toInt();
    //m_iLevel=xPP["clientLevel"].toInt();

    m_iLevel=1;
    if(m_iAP>10000) {m_iLevel=2;};
    if(m_iAP>30000) {m_iLevel=3;};
    if(m_iAP>70000) {m_iLevel=4;};
    if(m_iAP>150000) {m_iLevel=5;};
    if(m_iAP>300000) {m_iLevel=6;};
    if(m_iAP>600000) {m_iLevel=7;};
    if(m_iAP>1200000) {m_iLevel=8;};

    m_iXM=xPP["energy"].toInt();
    m_iXMMax=2000+m_iLevel*1000;
}

void DataCfg::TPlayer::Update(const QVariant& p_xVM)
{
    QVariantList xL=p_xVM.toList();
    if(xL.size()<3) {return;};
    QVariantMap xD=xL[2].toMap();
    QVariantMap xPP=xD["playerPersonal"].toMap();

    QVariantMap xM=m_xData.toMap();
    QVariantList axPE=xM["playerEntity"].toList();
    if(axPE.size()<3) {return;};
    QVariantMap xPE=axPE[2].toMap();
    QVariantMap xPPOrg=xPE["playerPersonal"].toMap();

    xPPOrg=MergeVarMaps(xPPOrg,xPP,QStringList());
    xPE["playerPersonal"]=xPPOrg;
    axPE[2]=xPE;
    xM["playerEntity"]=axPE;

    m_xData=xM;

    Load(m_xData);
}

void DataCfg::TItem::Load(const QVariant& p_xVM)
{
    TObj::Load(p_xVM);

    QVariantList xL=m_xData.toList();
    if(xL.size()<3) {return;};
    m_sGUID=xL[0].toString();
    m_iTime=xL[1].toLongLong();
    QVariantMap xD=xL[2].toMap();

    QVariantMap xLoc=xD["locationE6"].toMap();
    float fLat=xLoc["latE6"].toDouble()/1E6;
    float fLon=xLoc["lngE6"].toDouble()/1E6;
    m_xPos=QPointF(fLon,fLat);

    if(xD.contains("resourceWithLevels"))
    {
        QVariantMap xRWL=xD["resourceWithLevels"].toMap();
        m_iLevel=xRWL["level"].toInt();
        QString sType=xRWL["resourceType"].toString();
        m_eType=StringToType(sType);
        if(m_eType==IT_Media) {m_iLevel=0;}; //ignore media level
    };
    if(xD.contains("modResource"))
    {
        QVariantMap xMR=xD["modResource"].toMap();
        QString sType=xMR["resourceType"].toString();
        QString sRar=xMR["rarity"].toString();
        m_eType=StringToType(sType);
        m_iLevel=1;
        if(sRar=="RARE") {m_iLevel=2;};
        if(sRar=="VERY_RARE") {m_iLevel=3;};
    };
    if(xD.contains("flipCard"))
    {
        QVariantMap xMR=xD["flipCard"].toMap();
        QString sType=xMR["flipCardType"].toString();
        if(sType=="JARVIS") {m_eType=IT_Jarvis;};
        if(sType=="ADA") {m_eType=IT_Ada;};
    };

    if(xD.contains("portalCoupler"))
    {
        QVariantMap xMR=xD["portalCoupler"].toMap();
        m_sKeyPortalGUID=xMR["portalGuid"].toString();
        m_sKeyPortalName=xMR["portalTitle"].toString();
        m_eType=IT_Key;
        m_iLevel=0;
    };
    //if(sType=="EMP_BURSTER") {m_eType=IT_XMP;};

    if(m_eType==IT_Invalid)
    {
        qDebug() << "Unknown Item:" << p_xVM;
    }
}

DataCfg::ItemType DataCfg::TItem::StringToType(QString p_sType)
{
    if(p_sType=="RES_SHIELD") {return IT_Shield;};
    if(p_sType=="LINK_AMPLIFIER") {return IT_LinkAmp;};
    if(p_sType=="MULTIHACK") {return IT_Multihack;};
    if(p_sType=="HEATSINK") {return IT_HeatSink;};
    if(p_sType=="FORCE_AMP") {return IT_ForceAmp;};
    if(p_sType=="TURRET") {return IT_Turret;};
    if(p_sType=="EMP_BURSTER") {return IT_XMP;};
    if(p_sType=="EMITTER_A") {return IT_Resonator;};
    if(p_sType=="POWER_CUBE") {return IT_Cube;};
    if(p_sType=="MEDIA") {return IT_Media;};

    return IT_Invalid;
}

bool DataCfg::TInventory::Append(const QVariantList& p_xVM)
{
    if(p_xVM.count()<=0) {return false;};

    QVariantList xList=m_xData.toList();
    foreach(QVariant xV,p_xVM)
    {
        QVariantList xL=xV.toList();
        if(xL.size()<3) {continue;};
        QString sGUID=xL[0].toString();

        if(GetItem(sGUID)) {continue;};
        xList.append(xV);
    };
    m_xData=xList;

    Parse();

    return true;
}

void DataCfg::TInventory::Clear()
{
    m_xData=QVariantList();
    Parse();
}

void DataCfg::TInventory::Load(const QVariant& p_xVM)
{
    TObj::Load(p_xVM);
    Parse();
}

DataCfg::TItem* DataCfg::TInventory::GetItem(QString p_sGUID)
{
    for(int i=0;i<m_axItems.size();i++)
    {
        TItem& rxP=m_axItems[i];
        if(rxP.m_sGUID==p_sGUID) {return &rxP;};
    };
    return NULL;
}

QStringList DataCfg::TInventory::Fetch(ItemType p_eType,int p_iCount,int p_iLevel)
{
    QStringList asRet;
    for(int i=0;i<m_axItems.size()&&p_iCount>0;i++)
    {
        TItem& rxI=m_axItems[i];
        if(rxI.m_eType!=p_eType) {continue;};
        if(rxI.m_iLevel!=p_iLevel) {continue;};
        asRet.append(rxI.m_sGUID);
        p_iCount--;
    }
    return asRet;
}

QStringList DataCfg::TInventory::FetchKeysByPortal(QStringList p_asPortals)
{
    QStringList asRet;
    for(int i=0;i<m_axItems.size();i++)
    {
        TItem& rxI=m_axItems[i];
        if(rxI.m_eType!=IT_Key) {continue;};
        if(!p_asPortals.contains(rxI.m_sKeyPortalGUID)) {continue;};
        if(asRet.contains(rxI.m_sGUID)) {continue;};
        asRet.append(rxI.m_sGUID);
        p_asPortals.removeOne(rxI.m_sKeyPortalGUID);
        if(p_asPortals.size()<=0) {break;};
    }
    return asRet;

}

bool DataCfg::TInventory::DeleteEntities(QStringList& p_asEnts)
{
    QStringList asRem;

    forint(i,p_asEnts)
    {
        QString sE=p_asEnts[i];
        forint(j,m_axItems)
        {
            if(m_axItems[j].m_sGUID==sE)
            {
                qDebug() << "InvRem:" << sE;
                m_axItems.removeAt(j);
                asRem.append(sE);
                i--;
                break;
            };
        }
    };

    if(!asRem.count()) {return false;};
    QVariantList xL=m_xData.toList();
    forint(j,xL)
    {
        QVariantList aI=xL[j].toList();
        if(aI.size()<3) {continue;};
        QString sGuid=aI[0].toString();
        int iIdx=asRem.indexOf(sGuid);
        if(iIdx==-1) {continue;};
        xL.removeAt(j);
        j--;
    };
    m_xData=xL;
    Parse();
    return true;
}


void DataCfg::TInventory::Parse()
{
    m_axItems.clear();
    m_aiCountR.clear();
    m_aiCountX.clear();
    m_aiCountC.clear();
    m_aiCountS.clear();
    m_aiCountL.clear();
    m_aiCountU.clear();
    m_aiCountH.clear();
    m_aiCountF.clear();
    m_aiCountT.clear();
    for(int i=0;i<8;i++)
    {
        m_aiCountR.append(0);
        m_aiCountX.append(0);
        m_aiCountC.append(0);
    };
    for(int i=0;i<3;i++)
    {
        m_aiCountS.append(0);
        m_aiCountL.append(0);
        m_aiCountU.append(0);
        m_aiCountH.append(0);
        m_aiCountF.append(0);
        m_aiCountT.append(0);
    };
    m_iCountK=0;
    m_iCountJ=0;
    m_iCountA=0;
    m_iCountM=0;

    QVariantList xL=m_xData.toList();
    foreach(QVariant xV,xL)
    {
        TItem xI;
        xI.Load(xV);
        m_axItems.append(xI);
    };

    for(int i=0;i<m_axItems.size();i++)
    {
        TItem& rxI=m_axItems[i];
        if(rxI.m_eType==IT_Key) {m_iCountK++;};
        if(rxI.m_eType==IT_Jarvis) {m_iCountJ++;};
        if(rxI.m_eType==IT_Ada) {m_iCountA++;};
        if(rxI.m_eType==IT_Media) {m_iCountM++;};
        if(rxI.m_eType==IT_Resonator) {m_aiCountR[rxI.m_iLevel-1]++;};
        if(rxI.m_eType==IT_XMP) {m_aiCountX[rxI.m_iLevel-1]++;};
        if(rxI.m_eType==IT_Cube) {m_aiCountC[rxI.m_iLevel-1]++;};
        if(rxI.m_eType==IT_Shield) {m_aiCountS[rxI.m_iLevel-1]++;};
        if(rxI.m_eType==IT_LinkAmp) {m_aiCountL[rxI.m_iLevel-1]++;};
        if(rxI.m_eType==IT_Multihack) {m_aiCountU[rxI.m_iLevel-1]++;};
        if(rxI.m_eType==IT_HeatSink) {m_aiCountH[rxI.m_iLevel-1]++;};
        if(rxI.m_eType==IT_ForceAmp) {m_aiCountF[rxI.m_iLevel-1]++;};
        if(rxI.m_eType==IT_Turret) {m_aiCountT[rxI.m_iLevel-1]++;};
    }
}

void DataCfg::TGameEnts::Load(const QVariant& p_xVM)
{
    TObj::Load(p_xVM);
    Parse();
}

void DataCfg::TField::Load(const QVariant& p_xVM)
{
    TObj::Load(p_xVM);
    QVariantList xL=m_xData.toList();
    if(xL.size()<3) {return;};
    m_sGUID=xL[0].toString();
    m_iTime=xL[1].toLongLong();
    QVariantMap xD=xL[2].toMap();

    QVariantMap xCT=xD["controllingTeam"].toMap();
    m_eTeam=(xCT["team"].toString()=="ALIENS")?T_Aliens:T_Humans;

    QVariantMap xReg=xD["capturedRegion"].toMap();
    QVariantMap xLoc=xReg["vertexA"].toMap()["location"].toMap();
    float fLat=xLoc["latE6"].toDouble()/1E6;
    float fLon=xLoc["lngE6"].toDouble()/1E6;
    m_vA=QPointF(fLon,fLat);
    xLoc=xReg["vertexB"].toMap()["location"].toMap();
    fLat=xLoc["latE6"].toDouble()/1E6;
    fLon=xLoc["lngE6"].toDouble()/1E6;
    m_vB=QPointF(fLon,fLat);
    xLoc=xReg["vertexC"].toMap()["location"].toMap();
    fLat=xLoc["latE6"].toDouble()/1E6;
    fLon=xLoc["lngE6"].toDouble()/1E6;
    m_vC=QPointF(fLon,fLat);
}

int DataCfg::TPortal::GetChargePrc()
{
    if(m_iXMMax<=0) {return 0;};
    if(m_iXM>=m_iXMMax) {return 100;}
    float fCharge=float(m_iXM)/m_iXMMax;
    int iC=int(fCharge*100+0.5);
    if(iC>99) {iC=99;};
    return iC;
}

int DataCfg::TPortal::GetUpgradeModSlot()
{
    int iMaskFree=1|2|4|8;
    forint(i,m_axMods)
    {
        TMod& xM=m_axMods[i];
        iMaskFree&=~(1<<xM.m_iSlot);
    };
    for(int i=0;i<4;i++)
    {
        if(iMaskFree&(1<<i)) {return i;};
    };
    return -1;
}

bool DataCfg::TPortal::CanDeployResonator(QString p_sPlayer,int p_iLevel)
{
    if(m_axResonators.count()>=8) {return false;};
    int iCntOL=0;
    forint(i,m_axResonators)
    {
        TResonator& xR=m_axResonators[i];
        if(xR.m_sOwner==p_sPlayer&&xR.m_iLevel==p_iLevel) {iCntOL++;};
    };
    if(p_iLevel==1&&iCntOL>=8) {return false;};
    if(p_iLevel==2&&iCntOL>=4) {return false;};
    if(p_iLevel==3&&iCntOL>=4) {return false;};
    if(p_iLevel==4&&iCntOL>=4) {return false;};
    if(p_iLevel==5&&iCntOL>=2) {return false;};
    if(p_iLevel==6&&iCntOL>=2) {return false;};
    if(p_iLevel==7&&iCntOL>=1) {return false;};
    if(p_iLevel==8&&iCntOL>=1) {return false;};
    return true;
}

int DataCfg::TPortal::GetUpgradeResonatorSlot(QString p_sPlayer,int p_iLevel)
{
    if(m_axResonators.count()<8) {return -1;};
    int iCntOL=0;
    int iSmall=-1;
    forint(i,m_axResonators)
    {
        TResonator& xR=m_axResonators[i];
        if(xR.m_sOwner==p_sPlayer&&xR.m_iLevel==p_iLevel) {iCntOL++;};
        if(xR.m_iLevel<p_iLevel) {iSmall=i;};
    };
    if(p_iLevel==1&&iCntOL>=8) {return -1;};
    if(p_iLevel==2&&iCntOL>=4) {return -1;};
    if(p_iLevel==3&&iCntOL>=4) {return -1;};
    if(p_iLevel==4&&iCntOL>=4) {return -1;};
    if(p_iLevel==5&&iCntOL>=2) {return -1;};
    if(p_iLevel==6&&iCntOL>=2) {return -1;};
    if(p_iLevel==7&&iCntOL>=1) {return -1;};
    if(p_iLevel==8&&iCntOL>=1) {return -1;};
    return iSmall;
}

void DataCfg::TPortal::Load(const QVariant& p_xVM)
{
    TObj::Load(p_xVM);
    QVariantList xL=m_xData.toList();
    if(xL.size()<3) {return;};
    m_sGUID=xL[0].toString();
    m_iTime=xL[1].toLongLong();
    QVariantMap xD=xL[2].toMap();

    QVariantMap xCT=xD["controllingTeam"].toMap();
    QString sTeam=xCT["team"].toString();
    m_eTeam=(sTeam=="ALIENS")?T_Aliens:((sTeam=="RESISTANCE")?T_Humans:T_Neutral);

    QVariantMap xLoc=xD["locationE6"].toMap();
    float fLat=xLoc["latE6"].toDouble()/1E6;
    float fLon=xLoc["lngE6"].toDouble()/1E6;
    m_xPos=QPointF(fLon,fLat);

    static int aiXMResLvl[]={1000,1500,2000,2500,3000,4000,5000,6000};

    m_iLevel=0;
    m_iXM=0;
    m_iXMMax=0;
    m_axResonators.clear();
    QVariantList axResos=xD["resonatorArray"].toMap()["resonators"].toList();
    for(int i=0;i<axResos.size();i++)
    {
        if(axResos[i].isNull()) {continue;};
        QVariantMap xRes=axResos[i].toMap();
        int iLevel=xRes["level"].toInt();
        int iXM=xRes["energyTotal"].toInt();
        m_iLevel+=iLevel;
        m_iXM+=iXM;
        m_iXMMax+=aiXMResLvl[iLevel-1];

        TResonator xR;
        xR.m_iLevel=xRes["level"].toInt();
        xR.m_iDistance=xRes["distanceToPortal"].toInt();
        xR.m_iXM=xRes["energyTotal"].toInt();
        xR.m_iXMMax+=aiXMResLvl[xR.m_iLevel-1];
        xR.m_iSlot=xRes["slot"].toInt();
        xR.m_sOwner=xRes["ownerGuid"].toString();
        m_axResonators.append(xR);
    };
    m_iLevel/=8;


    QVariantMap xPO=xD["portalV2"].toMap();
    m_sName=xPO["descriptiveText"].toMap()["TITLE"].toString();
    QVariantList axEdges=xPO["linkedEdges"].toList();
    for(int i=0;i<axEdges.size();i++)
    {
        QVariantMap xE=axEdges[i].toMap();
        if(!xE["isOrigin"].toBool()) {continue;};
        m_asEdges.append(xE["otherPortalGuid"].toString());
    };

    m_asMods.clear();
    QVariantList axMods=xPO["linkedModArray"].toList();
    for(int i=0;i<axMods.size();i++)
    {
        if(axMods[i].isNull()) {continue;};
        QVariantMap xE=axMods[i].toMap();
        QString sType=xE["type"].toString();
        QString sRarity=xE["rarity"].toString();

        TMod xM;
        xM.m_iSlot=i;
        xM.m_eType=TItem::StringToType(sType);
        m_axMods.append(xM);


        QString sModMini;
        if(sType=="RES_SHIELD") {sModMini="S";};
        if(sType=="LINK_AMPLIFIER") {sModMini="L";};
        if(sType=="MULTIHACK") {sModMini="U";};
        if(sType=="HEATSINK") {sModMini="H";};
        if(sType=="FORCE_AMP") {sModMini="F";};
        if(sType=="TURRET") {sModMini="T";};
        if(sModMini.isEmpty()) {sModMini="?";};

        if(sRarity=="COMMON") {sModMini+="-";};
        //if(sRarity=="RARE") {sModMini+="R";};
        if(sRarity=="VERY_RARE") {sModMini+="+";};
        m_asMods.append(sModMini);
    };
}

DataCfg::TPortal* DataCfg::TGameEnts::GetPortal(QString p_sGUID)
{
    for(int i=0;i<m_axPortals.size();i++)
    {
        TPortal& rxP=m_axPortals[i];
        if(rxP.m_sGUID==p_sGUID) {return &rxP;};
    };
    return NULL;
}

void DataCfg::TGameEnts::DeleteEntities(QStringList& p_asEnts)
{
    forint(i,p_asEnts)
    {
        QString sE=p_asEnts[i];
        forint(j,m_axItems)
        {
            if(m_axItems[j].m_sGUID==sE)
            {
                qDebug() << "ObjRem:" << sE;
                m_axItems.removeAt(j);
                i--;
                break;
            };
        }
    };
}

QStringList DataCfg::TGameEnts::FetchItems(QPointF p_xPos)
{
    QStringList asRet;
    for(int i=0;i<m_axItems.size();i++)
    {
        TItem& rxI=m_axItems[i];
        float fD=S2::PosDistance(rxI.m_xPos,p_xPos);
        if(fD>35) {continue;};
        //qDebug() << fD;
        asRet.append(rxI.m_sGUID);
    };
    return asRet;
}

QStringList DataCfg::TGameEnts::FetchPortals(QPointF p_xPos)
{
    QStringList asRet;
    for(int i=0;i<m_axPortals.size();i++)
    {
        TPortal& rxI=m_axPortals[i];
        float fD=S2::PosDistance(rxI.m_xPos,p_xPos);
        if(fD>35) {continue;};
        //qDebug() << fD;
        asRet.append(rxI.m_sGUID);
    };
    return asRet;
}

bool DataCfg::TGameEnts::Update(const QVariantList& p_xVM)
{
    QVariantList axCur=m_xData.toList();
    bool bRet=false;

    foreach(QVariant xV,p_xVM)
    {
        QVariantList xL=xV.toList();
        if(xL.size()<3) {continue;};
        QString sGuid=xL[0].toString();

        forint(i,axCur)
        {
            QVariantList xCL=axCur[i].toList();
            if(xCL.size()<3) {continue;};
            QString sGuidCur=xCL[0].toString();
            if(sGuid!=sGuidCur) {continue;};

            bRet=true;
            axCur[i]=xL;
        }
    };

    if(bRet)
    {
        m_xData=axCur;
        Parse();
    };

    return bRet;
}

void DataCfg::TGameEnts::Parse()
{
    m_axPortals.clear();
    m_axItems.clear();
    m_axFields.clear();

    QVariantList xL=m_xData.toList();
    //qDebug() << xL.count();
    foreach(QVariant xV,xL)
    {
        QVariantList xL=xV.toList();
        if(xL.size()<3) {continue;};
        QVariantMap xM=xL[2].toMap();
        if(xM.contains("portalV2"))
        {
            TPortal xI;
            xI.Load(xV);
            m_axPortals.append(xI);
            continue;
        };
        if(xM.contains("capturedRegion"))
        {
            TField xI;
            xI.Load(xV);
            m_axFields.append(xI);
            continue;
        };

        if(xM.contains("resourceWithLevels")||xM.contains("flipCard")
                ||xM.contains("portalCoupler")||xM.contains("modResource"))
        {
            TItem xI;
            xI.Load(xV);
            m_axItems.append(xI);
            continue;
        };
    };
}

void DataCfg::TEnergy::Load(const QVariant& p_xVM)
{
    TObj::Load(p_xVM);
    m_asGlobs=p_xVM.toStringList();
}

QVariant DataCfg::TEnergy::Export()
{
    return m_asGlobs;
}

int DataCfg::TEnergy::EnergySum()
{
    int iR=0;
    for(int i=0;i<m_asGlobs.count();i++)
    {
        iR+=GlobValue(m_asGlobs[i]);
    }
    return iR;
}

QStringList DataCfg::TEnergy::FetchFrom(QStringList p_asXM,int p_iAmount)
{
    QStringList asRet;
    QStringList asXM=p_asXM;
    while(p_iAmount>0&&asXM.count()>0)
    {
        int iIdx=qrand()%asXM.count();
        QString sXM=asXM.takeAt(iIdx);
        p_iAmount-=GlobValue(sXM);
        asRet.append(sXM);
    };
    return asRet;
}

QStringList DataCfg::TInfluxData::FetchMultiXM(int p_iAmount)
{
    QStringList asXM;
    forint(i,m_axPlaces)
    {
        TEnergy& rxE=m_axPlaces[i].m_xEnergy;
        if(rxE.EnergySum()<20000) {continue;}; //not enough for sneakyness
        asXM+=rxE.m_asGlobs;
    };

    QStringList asRet;
    while(p_iAmount>0&&asXM.count()>0)
    {
        int iIdx=qrand()%asXM.count();
        QString sXM=asXM.takeAt(iIdx);
        p_iAmount-=DataCfg::TEnergy::GlobValue(sXM);
        if(!asRet.contains(sXM)) {asRet.append(sXM);};
    };

    forint(j,asRet)
    {
        QString sE=asRet[j];
        forint(i,m_axPlaces)
        {
            TEnergy& rxE=m_axPlaces[i].m_xEnergy;
            rxE.m_asGlobs.removeOne(sE);
        };
    };

    return asRet;

}

int DataCfg::TEnergy::GlobValue(QString p_sGlob)
{
    char c0=p_sGlob[30].toLatin1();
    char c1=p_sGlob[31].toLatin1();

    static const QString s="0123456789abcdef";
    int i0=s.indexOf(c0);
    int i1=s.indexOf(c1);

    return i0*16+i1;
}

QPointF DataCfg::TEnergy::GlobPos(QString p_sGlob)
{
    p_sGlob=p_sGlob.left(16);
    quint64 iCellID=p_sGlob.toLongLong(0,16);
    //qDebug()<< S2::CellIDLevel(iCellID);
    return S2::CellIDToPos(iCellID);
}

QString DataCfg::TUserCfg::GetDeviceID()
{
    if(m_sDeviceID.size()!=16)
    {
        QUuid xGUID=QUuid::createUuid();
        quint64 iID=*((quint64*)(&xGUID.data4[0]));
        m_sDeviceID=QString::number(iID,16);
    };
    return m_sDeviceID;
}

void DataCfg::TUserCfg::Load(const QVariant& p_xVM)
{
    TObj::Load(p_xVM);

    m_iCurZ=16;
    QVariantMap xM=m_xData.toMap();
    if(xM.contains("curPos"))
    {
        QVariantMap xPos=xM["curPos"].toMap();
        m_xPos.setX(xPos["x"].toFloat());
        m_xPos.setY(xPos["y"].toFloat());
        m_iCurZ=xPos["z"].toInt();
    };
    m_sLastInvSync=xM["InvSyncT"].toString();
    m_sDeviceID=xM["DeviceID"].toString();

    if(m_sLastInvSync.length()<2){m_sLastInvSync="0";}
    m_iLastActionTime=xM["LastActionTime"].toLongLong();
    if(xM.contains("LastActionPos"))
    {
        QVariantMap xPos=xM["LastActionPos"].toMap();
        m_xLastActionPos.setX(xPos["x"].toFloat());
        m_xLastActionPos.setY(xPos["y"].toFloat());
    };

    ResolveCookie();
}

void DataCfg::TInfluxCfg::Load(const QVariant& p_xVM)
{
    TObj::Load(p_xVM);
    QVariantMap xM=m_xData.toMap();

    m_axPlaces.clear();
    QVariantList axPl=xM["Places"].toList();
    forint(i,axPl)
    {
        QVariantMap xPl=axPl[i].toMap();
        TPlace xP;
        xP.m_sName=xPl["Name"].toString();
        xP.m_sPos=xPl["Pos"].toString();
        xP.m_sMacro=xPl["Macro"].toString();
        xP.m_fSlurpPrio=xPl["SlurpPriority"].toFloat();
        m_axPlaces.append(xP);
    };
}

QVariant DataCfg::TInfluxCfg::Export()
{
    QVariantMap xRet;
    QVariantList axP;
    forint(i,m_axPlaces)
    {
        QVariantMap xPl;
        xPl["Name"]=m_axPlaces[i].m_sName;
        xPl["Pos"]=m_axPlaces[i].m_sPos;
        xPl["Macro"]=m_axPlaces[i].m_sMacro;
        xPl["SlurpPriority"]=m_axPlaces[i].m_fSlurpPrio;
        axP.append(xPl);
    };
    xRet["Places"]=axP;
    return xRet;
}


void DataCfg::TInfluxData::Load(const QVariant& p_xVM)
{
    TObj::Load(p_xVM);
    QVariantMap xM=m_xData.toMap();
    m_xEnergyCache.Load(xM["EnergyCache"]);
    m_axPlaces.clear();
    QVariantList axPl=xM["Places"].toList();
    forint(i,axPl)
    {
        QVariantMap xPl=axPl[i].toMap();
        TPlaceXM xP;
        xP.m_sName=xPl["Name"].toString();
        xP.m_xEnergy.Load(xPl["Energy"]);
        m_axPlaces.append(xP);
    };
}

QVariant DataCfg::TInfluxData::Export()
{
    QVariantMap xRet;
    QVariantList axP;
    forint(i,m_axPlaces)
    {
        QVariantMap xPl;
        xPl["Name"]=m_axPlaces[i].m_sName;
        xPl["Energy"]=m_axPlaces[i].m_xEnergy.Export();
        axP.append(xPl);
    };
    xRet["Places"]=axP;
    xRet["EnergyCache"]=m_xEnergyCache.Export();
    return xRet;
}

DataCfg::TInfluxCfg::TPlace* DataCfg::TInfluxCfg::GetPlace(QString p_sName)
{
    forint(i,m_axPlaces)
    {
        if(m_axPlaces[i].m_sName==p_sName) {return &m_axPlaces[i];}
    };
    return NULL;
}

DataCfg::TInfluxData::TPlaceXM* DataCfg::TInfluxData::GetPlace(QString p_sName)
{
    forint(i,m_axPlaces)
    {
        if(m_axPlaces[i].m_sName==p_sName) {return &m_axPlaces[i];}
    };
    return NULL;
}

void DataCfg::TInfluxCfg::AddPlace(QString p_sName,QString p_sPos,float p_fSlurpPrio)
{
    TPlace* pxP=GetPlace(p_sName);
    if(!pxP) {m_axPlaces.append(TPlace());pxP=&m_axPlaces.last();};
    pxP->m_sName=p_sName;
    pxP->m_sPos=p_sPos;
    pxP->m_fSlurpPrio=p_fSlurpPrio;
}

void DataCfg::TInfluxCfg::RemPlace(QString p_sName)
{
    forint(i,m_axPlaces)
    {
        if(m_axPlaces[i].m_sName==p_sName) {m_axPlaces.removeAt(i);return;}
    };
}

void DataCfg::TInfluxData::RemPlace(QString p_sName)
{
    forint(i,m_axPlaces)
    {
        if(m_axPlaces[i].m_sName==p_sName) {m_axPlaces.removeAt(i);return;}
    };
}

void DataCfg::TInfluxCfg::SetPlaceMacro(QString p_sName,QString p_sMacro)
{
    TPlace* pxP=GetPlace(p_sName);
    if(!pxP) {return;}
    pxP->m_sMacro=p_sMacro;
}

QString DataCfg::TInfluxCfg::GetPlaceMacro(QString p_sName)
{
    TPlace* pxP=GetPlace(p_sName);
    if(!pxP) {return QString();}
    return pxP->m_sMacro;
}

QVariant DataCfg::TUserCfg::Export()
{
    QVariantMap xM=TObj::Export().toMap();

    QVariantMap xPos;
    xPos["x"]=m_xPos.x();
    xPos["y"]=m_xPos.y();
    xPos["z"]=m_iCurZ;
    xM["curPos"]=xPos;
    xM["InvSyncT"]=m_sLastInvSync;
    xM["LastActionTime"]=m_iLastActionTime;
    xM["DeviceID"]=m_sDeviceID;

    xPos=QVariantMap();
    xPos["x"]=m_xLastActionPos.x();
    xPos["y"]=m_xLastActionPos.y();

    xM["LastActionPos"]=xPos;
    return xM;
}

void DataCfg::TUserCfg::ResolveCookie()
{
    QStringList asC=GetLoginCookies().toStringList();
    foreach(QString sC,asC)
    {
        if(sC.startsWith("SACSID="))
        {
            m_sAccCookie=sC;
            break;
        };
    };
}

void DataCfg::TUserCfg::SetLoginCookies(QVariant p_xV)
{
    QVariantMap xM=m_xData.toMap();
    xM["LoginCookies"]=p_xV;
    m_xData=xM;
    ResolveCookie();
}

QVariant DataCfg::TUserCfg::GetLoginCookies()
{
    return m_xData.toMap()["LoginCookies"];
}

void DataCfg::Init()
{
    QString sFileName=InfluxWnd::ms_pxInst->m_sPathUser+"UserCfg.json";
    LoadObj(m_xUserCfg,sFileName);
    sFileName=InfluxWnd::ms_pxInst->m_sPathUser+"Player.json";
    LoadObj(m_xPlayer,sFileName);
    sFileName=InfluxWnd::ms_pxInst->m_sPathUser+"Inventory.json";
    LoadObj(m_xInventory,sFileName);
    sFileName=InfluxWnd::ms_pxInst->m_sPathUser+"GameEnts.json";
    LoadObj(m_xGameEnts,sFileName);
    sFileName=InfluxWnd::ms_pxInst->m_sPathUser+"Energy.json";
    LoadObj(m_xEnergy,sFileName);
    sFileName=InfluxWnd::ms_pxInst->m_sPathUser+"meta/Influx.json";
    LoadObj(m_xInfluxCfg,sFileName);
    sFileName=InfluxWnd::ms_pxInst->m_sPathUser+"meta/Data.json";
    LoadObj(m_xInfluxData,sFileName);
}

void DataCfg::Shut()
{
    SaveObj(m_xUserCfg);
    SaveObj(m_xPlayer);
    SaveObj(m_xInventory);
    SaveObj(m_xGameEnts);
    SaveObj(m_xEnergy);
    SaveObj(m_xInfluxCfg);
    SaveObj(m_xInfluxData);
}

void DataCfg::Logout()
{
    m_xUserCfg.m_xData.clear();
    m_xPlayer.m_xData.clear();
    m_xInventory.m_xData.clear();
    m_xGameEnts.m_xData.clear();
    m_xEnergy.m_xData.clear();

    Shut();
}

void DataCfg::SaveSnapshot()
{
    Shut();

    QString p_sName=m_xPlayer.m_sNick;
    p_sName.remove(QRegExp("[^a-zA-Z\\d\\s]"));
    if(p_sName.isEmpty()) {return;};

    QString sPSnap=InfluxWnd::ms_pxInst->m_sPathUser+"_"+p_sName+"/";
    QString sDir=InfluxWnd::ms_pxInst->m_sPathUser;
    QDir().mkpath(sPSnap);
    QDir xSnap(sPSnap);
    QStringList asF=xSnap.entryList(QDir::Files);
    foreach(QString s,asF)
    {
        QFile::remove(sPSnap+s);
    };

    QDir xCur(sDir);
    QStringList asC=xCur.entryList(QDir::Files);
    foreach(QString s,asC)
    {
        QFile::copy(sDir+s,sPSnap+s);
    };
}

bool DataCfg::LoadSnapshot(QString p_sName)
{
    QString sPSnap=InfluxWnd::ms_pxInst->m_sPathUser+"_"+p_sName+"/";
    QString sDir=InfluxWnd::ms_pxInst->m_sPathUser;
    QDir xSnap(sPSnap);
    if(!xSnap.exists()) {return false;};

    SaveSnapshot();
    Logout();

    QDir xCur(sDir);
    QStringList asC=xCur.entryList(QDir::Files);
    foreach(QString s,asC)
    {
        QFile::remove(sDir+s);
    };

    QStringList asF=xSnap.entryList(QDir::Files);
    foreach(QString s,asF)
    {
        QFile::copy(sPSnap+s,sDir+s);
    };

    Init();

    return true;
}

QStringList DataCfg::GetSnapshots()
{
    QStringList asRet;

    QString sDir=InfluxWnd::ms_pxInst->m_sPathUser;
    QDir xCur(sDir);
    QStringList asC=xCur.entryList(QDir::Dirs);
    foreach(QString s,asC)
    {
        if(s.startsWith("_")) {asRet.append(s.mid(1));};
    };
    return asRet;
}

bool DataCfg::LoadObj(TObj& p_xO,QString p_sFileName)
{
    p_xO.m_sFileName=p_sFileName;
    QFile xF(p_sFileName);
    if(!xF.open(QFile::ReadOnly))
    {
        //try backup
        QString sFB=p_sFileName.replace(".json",".jsbk",Qt::CaseInsensitive);
        xF.setFileName(sFB);
        if(!xF.open(QFile::ReadOnly))
        {
            return false;
        };
    };
    QString sData(QString::fromUtf8(xF.readAll()));
    QString sErr;
    QVariant xV=QtJson::Json::parse(sData,sErr);
    if(sErr.length()>0) {qDebug() << sErr;return false;};
    p_xO.Load(xV);
    return true;
}

QString DataCfg::SaveObj(TObj& p_xO)
{
    QString sFileName=p_xO.m_sFileName;
    if(sFileName.isEmpty()) {return "missing file:"+sFileName;}

    QVariant xUC=p_xO.Export();

    QString sErr;
    QByteArray xRes=QtJson::Json::serialize(xUC,sErr,0);
    if(sErr.length()>0)
    {
        qDebug() << "Error:" << sErr << "saving:" << sFileName;
        return sErr;
    };

    //create backup
    QString sFNew=sFileName;
    sFNew=sFNew.replace(".json",".jsbk",Qt::CaseInsensitive);
    QFile::remove(sFNew);
    QFile::copy(sFileName,sFNew);

    //copy on write for main config!!!
    QString sFWrt=sFileName;
    sFWrt=sFWrt.replace(".json",".jso_",Qt::CaseInsensitive);
    QFile::remove(sFWrt);

    QFile xFile(sFWrt);
    if(!xFile.open(QIODevice::WriteOnly)) {return tr("Can't open File: ")+sFileName;};
    xFile.write(xRes);
    xFile.close();
    QFile::remove(sFileName);
    QFile::copy(sFWrt,sFileName);
    QFile::remove(sFWrt);
    return QString();
}

QString DataCfg::SaveUserCfg()
{
    return SaveObj(m_xUserCfg);
}

DataCfg::TMapPos DataCfg::ParseURLPos(QString p_sURL)
{
    TMapPos xRet;
    int iZoom=16;

    int iLL=p_sURL.lastIndexOf("ll=");
    if(iLL<0) {return xRet;}
    p_sURL=p_sURL.mid(iLL+3);
    int iZ=p_sURL.lastIndexOf("&z=");
    if(iZ>0)
    {
        QString sZ=p_sURL.mid(iZ+3);
        iZoom=sZ.toInt();
        p_sURL=p_sURL.left(iZ);
    };

    QStringList asPos=p_sURL.split(",");
    if(asPos.size()!=2) {return xRet;}
    xRet.m_xLL.setY(asPos[0].toFloat());
    xRet.m_xLL.setX(asPos[1].toFloat());
    xRet.m_iZ=iZoom;
    return xRet;
}

