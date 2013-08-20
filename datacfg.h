#ifndef DATACFG_H
#define DATACFG_H

#include "pch.h"

class DataCfg : public QObject
{
    Q_OBJECT
public:

    enum Team
    {
        T_Aliens,
        T_Humans,
        T_Neutral,
        T_Invalid=-1
    };

    enum ItemType
    {
        IT_Resonator,
        IT_XMP,
        IT_Key,
        IT_Cube,
        IT_Jarvis,
        IT_Ada,
        IT_Media,
        IT_Shield,      //>=IT_Shield -> Mod
        IT_Turret,
        IT_LinkAmp,
        IT_Multihack,
        IT_HeatSink,
        IT_ForceAmp,
        IT_Invalid=-1,
    };

    struct TMapPos
    {
        QPointF m_xLL;
        int m_iZ;
        TMapPos() {m_iZ=-1;}
        bool IsValid() const {return m_iZ!=-1;}
    };

    struct TObj
    {
    public:
        virtual ~TObj() {m_iTime=0;}

        QString m_sGUID;
        quint64 m_iTime;
        QPointF m_xPos;
        QString m_sFileName;
        QVariant m_xData;
        virtual void Load(const QVariant& p_xVM);
        virtual QVariant Export();
    };
    struct TPlayer : public TObj
    {
    public:
        bool m_bCanPlay;
        QString m_sNick;
        QString m_sXsrfToken;
        Team m_eTeam;

        int m_iAP;
        int m_iLevel;
        int m_iXM;
        int m_iXMMax;

        TPlayer() {m_bCanPlay=false;m_iAP=0;m_iLevel=0;m_iXM=0;m_iXMMax=0;m_eTeam=T_Invalid;}
        void Load(const QVariant& p_xVM);
        void Update(const QVariant& p_xVM);
    };
    struct TItem : public TObj
    {
    public:
        int m_iLevel;
        ItemType m_eType;
        QString m_sKeyPortalGUID;
        QString m_sKeyPortalName;
        TItem() {m_iTime=m_iLevel=0;m_eType=IT_Invalid;}

        static ItemType StringToType(QString p_sType);
        void Load(const QVariant& p_xVM);
    };
    struct TInventory : public TObj
    {
    public:
        QList<TItem> m_axItems;

        QList<int> m_aiCountR;
        QList<int> m_aiCountX;
        QList<int> m_aiCountC;
        QList<int> m_aiCountS;
        QList<int> m_aiCountL;
        QList<int> m_aiCountU;
        QList<int> m_aiCountH;
        QList<int> m_aiCountF;
        QList<int> m_aiCountT;
        int m_iCountK;
        int m_iCountJ;
        int m_iCountA;
        int m_iCountM;

        void Clear();
        void Load(const QVariant& p_xVM);
        bool Append(const QVariantList &p_xVM);
        void Parse();

        TItem* GetItem(QString p_sGUID);
        QStringList Fetch(ItemType p_eType,int p_iCount,int p_iLevel);
        QStringList FetchKeysByPortal(QStringList p_asPortals);
        bool DeleteEntities(QStringList& p_asEnts);
    };
    struct TPortal : public TObj
    {
    public:
        struct TResonator
        {
            int m_iDistance;
            int m_iXM;
            int m_iXMMax;
            int m_iLevel;
            int m_iSlot;
            QString m_sOwner;
        };
        struct TMod
        {
            ItemType m_eType;
            int m_iSlot;
        };

        Team m_eTeam;
        int m_iLevel;
        int m_iXM;
        int m_iXMMax;
        QString m_sName;
        QStringList m_asEdges;
        QList<TResonator> m_axResonators;
        QList<TMod> m_axMods;
        QStringList m_asMods;

        TPortal() {m_eTeam=T_Invalid;m_iLevel=0;m_iXM=0;m_iXMMax=0;}
        void Load(const QVariant& p_xVM);
        bool CanDeployResonator(QString p_sPlayer,int p_iLevel);
        int GetUpgradeResonatorSlot(QString p_sPlayer, int p_iLevel);
        int GetUpgradeModSlot();
        int GetChargePrc();
    };
    struct TField : public TObj
    {
    public:
        Team m_eTeam;
        QPointF m_vA;
        QPointF m_vB;
        QPointF m_vC;

        TField() {m_eTeam=T_Invalid;}
        void Load(const QVariant& p_xVM);
    };
    struct TGameEnts : public TObj
    {
    public:
        QList<TPortal> m_axPortals;
        QList<TItem> m_axItems;
        QList<TField> m_axFields;

        TGameEnts() {}
        void Load(const QVariant& p_xVM);
        bool Update(const QVariantList& p_xVM);
        void Parse();
        QStringList FetchItems(QPointF p_xPos);
        QStringList FetchPortals(QPointF p_xPos);
        TPortal* GetPortal(QString p_sGUID);
        void DeleteEntities(QStringList& p_asEnts);
    };
    struct TEnergy : public TObj
    {
    public:
        QStringList m_asGlobs;
        void Load(const QVariant& p_xVM);
        QVariant Export();

        void Clear() {m_asGlobs.clear();}
        int EnergySum();
        static int GlobValue(QString p_sGlob);
        static QPointF GlobPos(QString p_sGlob);
        static QStringList FetchFrom(QStringList p_asXM,int p_iAmount);
        QStringList Fetch(int p_iAmount) {return FetchFrom(m_asGlobs,p_iAmount);}
    };
    struct TUserCfg : public TObj
    {
    public:
        QString m_sAccCookie;
        int m_iCurZ;
        QString m_sLastInvSync;
        QPointF m_xLastActionPos;
        quint64 m_iLastActionTime;
        QString m_sDeviceID;

        QString GetDeviceID();
        void SetLoginCookies(QVariant p_xV);
        QVariant GetLoginCookies();
        void Load(const QVariant& p_xVM);
        QVariant Export();
        void ResolveCookie();
    };
    struct TInfluxCfg : public TObj
    {
    public:
        struct TPlace
        {
            QString m_sName;
            QString m_sPos;
            QString m_sMacro;
            float m_fSlurpPrio;
        };

        QList<TPlace> m_axPlaces;
        TPlace* GetPlace(QString p_sName);
        void RemPlace(QString p_sName);
        void AddPlace(QString p_sName, QString p_sPos,float p_fSlurpPrio);
        void Load(const QVariant& p_xVM);
        void SetPlaceMacro(QString p_sName,QString p_sMacro);
        QString GetPlaceMacro(QString p_sName);
        QVariant Export();
    };
    struct TInfluxData : public TObj
    {
        struct TPlaceXM
        {
            QString m_sName;
            TEnergy m_xEnergy;
        };
        QList<TPlaceXM> m_axPlaces;
        TEnergy m_xEnergyCache; //cash in as often as we want
        TPlaceXM* GetPlace(QString p_sName);
        void RemPlace(QString p_sName);
        QStringList FetchMultiXM(int p_iAmount);

        void Load(const QVariant& p_xVM);
        QVariant Export();
    };

    DataCfg();
    void Init();
    void Shut();

    static DataCfg* Get() {return ms_pxInst;}

    static TMapPos ParseURLPos(QString p_sURL);

    QString SaveUserCfg();
    QString SaveObj(TObj &p_xO);
    bool LoadObj(TObj &p_xO, QString p_sFileName);
    void Logout();

    void SaveSnapshot();
    QStringList GetSnapshots();
    bool LoadSnapshot(QString p_sName);

    TUserCfg m_xUserCfg;
    TPlayer m_xPlayer;
    TInventory m_xInventory;
    TGameEnts m_xGameEnts;
    TEnergy m_xEnergy;
    TInfluxCfg m_xInfluxCfg;
    TInfluxData m_xInfluxData;

    static QVariantMap MergeVarMaps(QVariantMap p_xA, QVariantMap p_xB, QStringList p_asIgnore);
private:
    static DataCfg* ms_pxInst;
};

#endif // DATACFG_H
