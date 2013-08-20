#include "influxwnd.h"
#include "ui_mainwindow.h"
#include "pch.h"
#include "dlglogin.h"
#include "util/json.h"
#include "util/s2.h"
#include "datacfg.h"
#include "api.h"
#include "logger/loggerwnd.h"
#include <QTableWidget>

#include "MapGraphics/MapGraphicsView.h"
#include "MapGraphics/MapGraphicsScene.h"
#include "MapGraphics/tileSources/OSMTileSource.h"
#include "MapGraphics/tileSources/CompositeTileSource.h"
#include "MapGraphics/CircleObject.h"
#include "MapGraphics/LineObject.h"
#include "MapGraphics/PolygonObject.h"

InfluxWnd* InfluxWnd::ms_pxInst=NULL;

InfluxWnd::InfluxWnd(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ms_pxInst=this;
    ui->setupUi(this);

    m_sPathUser=QDir::homePath()+"/Influx/";
    QDir().mkpath(m_sPathUser);
    QDir().mkpath(m_sPathUser+"meta");
    (new DataCfg())->Init();
    (new API(this))->Init();

    connect(&m_xTimerUpdatePortals,SIGNAL(timeout()),this,SLOT(OnUpdPortals_Defer()));
    m_xTimerUpdatePortals.setSingleShot(true);

    m_pxLogger=new LoggerWnd(ui->centralWidget);
    m_pxLogger->show();
    m_pxLogger->resize(10,20);

    //Setup the MapGraphics scene and view
    m_pxMapScene= new MapGraphicsScene(ui->centralWidget);
    m_pxMapView= new MapGraphicsView(m_pxMapScene,ui->centralWidget);
    m_pxMapView->lower();
    m_pxMapView->move(0,15);
    connect(m_pxMapView,SIGNAL(zoomLevelChanged(quint8)),this,SLOT(OnMapMoved()));
    connect(m_pxMapView,SIGNAL(moved()),this,SLOT(OnMapMoved()));
    //m_pxMapView->setStyleSheet("QWidget {border: 80px solid black;}");

    m_pxLblSpeed=new QLabel(m_pxMapView);
    m_pxLblSpeed->move(14,5);
    m_pxLblSpeed->setText("...");
    connect(&m_xTimerUpdateSpeed,SIGNAL(timeout()),this,SLOT(OnUpdSpeed()));
    m_xTimerUpdateSpeed.setInterval(250);
    m_xTimerUpdateSpeed.start();
    m_pxLineAction=new LineObject(Position(),Position(),0.35);
    m_pxMapScene->addObject(m_pxLineAction);

    //Setup some tile sources
    QSharedPointer<OSMTileSource> osmTiles(new OSMTileSource(OSMTileSource::MapQuestOSMTiles), &QObject::deleteLater);
    QSharedPointer<CompositeTileSource> composite(new CompositeTileSource(), &QObject::deleteLater);
    composite->addSourceBottom(osmTiles);
//    composite->addSourceBottom(aerialTiles);
//    composite->addSourceTop(gridTiles);
    m_pxMapView->setTileSource(composite);

    m_pxPlayer=new CircleObject(35.0,false,QColor("#097ec8"),NULL);
    m_pxPlayer->m_bPlayer=true;
    m_pxMapScene->addObject(m_pxPlayer);

    //http://www.ingress.com/intel?ll=52.515877,13.385765&z=13
    m_iZoom=16;
    m_xPos=QPointF(13.38680, 52.48420);
//    m_pxMapView->setZoomLevel(m_iZoom);

    //ui->tblItems->setHorizontalHeaderItem(0,new QTableWidgetItem("R"));
    //ui->tblItems->setHorizontalHeaderItem(1,new QTableWidgetItem("X"));
    //ui->tblItems->setHorizontalHeaderItem(2,new QTableWidgetItem("C"));

    connect(ui->btnInv,SIGNAL(clicked()),this,SLOT(OnBtnInv()));
    connect(ui->btnInvClear,SIGNAL(clicked()),this,SLOT(OnBtnInvClear()));
    connect(ui->btnScn,SIGNAL(clicked()),this,SLOT(OnBtnScn()));
    connect(ui->btnRecharge,SIGNAL(clicked()),this,SLOT(OnBtnRecharge()));
    connect(ui->btnDeployLo,SIGNAL(clicked()),this,SLOT(OnBtnDeployLo()));
    connect(ui->btnDeployHi,SIGNAL(clicked()),this,SLOT(OnBtnDeployHi()));
    connect(ui->btnHack1,SIGNAL(clicked()),this,SLOT(OnBtnHack()));
    connect(ui->btnHack10,SIGNAL(clicked()),this,SLOT(OnBtnHack()));
    connect(ui->btnHack20,SIGNAL(clicked()),this,SLOT(OnBtnHack()));
    connect(ui->btnHack40,SIGNAL(clicked()),this,SLOT(OnBtnHack()));
    connect(ui->btnHack60,SIGNAL(clicked()),this,SLOT(OnBtnHack()));
    connect(ui->btnXM,SIGNAL(clicked()),this,SLOT(OnBtnXM()));
    connect(ui->btnXMFlush,SIGNAL(clicked()),this,SLOT(OnBtnXMFlush()));
    connect(ui->btnXMSlurp,SIGNAL(clicked()),this,SLOT(OnBtnXMSlurp()));
    connect(ui->btnPickSel,SIGNAL(clicked()),this,SLOT(OnBtnPickSel()));
    connect(ui->btnPickAll,SIGNAL(clicked()),this,SLOT(OnBtnPickAll()));
    connect(ui->btnXMPLo,SIGNAL(clicked()),this,SLOT(OnBtnXMPLo()));
    connect(ui->btnXMPHi,SIGNAL(clicked()),this,SLOT(OnBtnXMPHi()));
    connect(ui->btnLinkQuery,SIGNAL(clicked()),this,SLOT(OnBtnLinkQuery()));
    connect(ui->btnLinkBuild,SIGNAL(clicked()),this,SLOT(OnBtnLinkBuild()));
    connect(ui->btnRec,SIGNAL(clicked()),this,SLOT(OnBtnRec()));
    connect(ui->btnPlay,SIGNAL(clicked()),this,SLOT(OnBtnPlay()));


    AssignBtnShortcut(ui->btnRecharge,QKeySequence("r"));
    AssignBtnShortcut(ui->btnXM,QKeySequence(" "));
    AssignBtnShortcut(ui->btnDeployHi,QKeySequence("d"));
    AssignBtnShortcut(ui->btnHack1,QKeySequence("h"));


    connect(ui->btnIC1,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIC2,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIC3,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIC4,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIC5,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIC6,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIC7,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIC8,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIR1,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIR2,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIR3,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIR4,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIR5,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIR6,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIR7,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIR8,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIX1,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIX2,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIX3,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIX4,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIX5,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIX6,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIX7,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIX8,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIS1,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIS2,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIS3,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIH1,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIH2,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIH3,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIU1,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIU2,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIU3,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIF1,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIF2,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIF3,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIT1,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIT2,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIT3,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIL1,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIL2,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIL3,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIKEY,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIMED,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIADA,SIGNAL(clicked()),this,SLOT(OnBtnItem()));
    connect(ui->btnIJAR,SIGNAL(clicked()),this,SLOT(OnBtnItem()));

    connect(API::Get(),SIGNAL(UpdInventory(QStringList)),this,SLOT(OnUpdInventory(QStringList)));
    connect(API::Get(),SIGNAL(UpdPortals()),this,SLOT(OnUpdPortals()));
    connect(API::Get(),SIGNAL(UpdPlayer()),this,SLOT(OnUpdPlayer()));
    connect(API::Get(),SIGNAL(UpdSlurp()),this,SLOT(UpdateConfig()));
    //connect(API::Get(),SIGNAL(OnRPCDone(QString)),this,SLOT(RPCDone(QString)));

    connect(ui->edtPos,SIGNAL(returnPressed()),this,SLOT(OnEdtPos()));
    connect(ui->btnUpdPlaces,SIGNAL(clicked()),this,SLOT(OnBtnUpdatePlaces()));
    connect(ui->btnAddPlace,SIGNAL(clicked()),this,SLOT(OnBtnPlaceAdd()));
    connect(ui->btnRemPlace,SIGNAL(clicked()),this,SLOT(OnBtnPlaceRem()));
    connect(ui->lstLocations,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(OnDblClickedLocation(QModelIndex)));
    connect(ui->lstLocations,SIGNAL(itemChanged(QListWidgetItem*)),this,SLOT(OnLocationItemChanged(QListWidgetItem*)));

    m_pxMenuLoad=new QMenu(this);
    ui->actionLoad_User->setMenu(m_pxMenuLoad);
    ui->prgMacro->setEnabled(false);

    UpdateConfig();
    OnUserLogin();
    UpdateUsers();
    OnResize();
}

InfluxWnd::~InfluxWnd()
{
    API::Get()->Shut();
    DataCfg::Get()->Shut();
    delete ui;
    ms_pxInst=NULL;
}

void InfluxWnd::OnUpdSpeed()
{
    quint64 iNow=QDateTime::currentMSecsSinceEpoch();
    quint64 iD=iNow-DataCfg::Get()->m_xUserCfg.m_iLastActionTime;
    if(iD<0) {return;};
    double fDist=S2::PosDistance(m_xPos,DataCfg::Get()->m_xUserCfg.m_xLastActionPos);
    double fSpeed=fDist/iD;
    fSpeed*=3600;

    m_pxLblSpeed->setText(QString::number(int(fSpeed))+" km/h");
    m_pxLineAction->setEndPointA(DataCfg::Get()->m_xUserCfg.m_xLastActionPos);
    m_pxLineAction->setEndPointB(m_xPos);

    if(!m_asXMPQ.isEmpty())
    {
        if(API::Get()->m_iRPCPending>0) {return;};
        if(DataCfg::Get()->m_xPlayer.m_iXM<=1000) {OnBtnXM();return;};
        API::Get()->RPCFireXMP(m_asXMPQ.takeFirst());
        return;
    };

    if(!MacroIsPlaying()) {return;};
    if(m_iMacroIdx>=m_asMacroPlay.size()) {OnBtnPlay();return;};
    if(API::Get()->m_iRPCPending>0) {return;};
    ui->prgMacro->setValue(m_iMacroIdx/2);
    int iMacroMode=ui->cmbMacroMode->currentIndex();
    //0=h&r 1=h 2=r

    QString sM=m_asMacroPlay[m_iMacroIdx];
    DataCfg::TPlayer& rxPlayer=DataCfg::Get()->m_xPlayer;

    static int iSpeedJitter=20;
    if(fSpeed>30+iSpeedJitter&&!sM.startsWith("m ")) {return;};
    iSpeedJitter=qrand()%20;

    m_iMacroIdx++;

    if(sM.startsWith("m "))
    {
        sM=sM.mid(2);
        qDebug()<<"move:"+sM;

        QStringList asPos=sM.split(",");
        if(asPos.size()!=2) {return;}
        QPointF vPos;
        vPos.setY(asPos[0].toFloat());
        vPos.setX(asPos[1].toFloat());
        m_xPos=vPos;
        m_pxMapView->centerOn(vPos);
        OnMapMoved();
        OnBtnXM();
    };
    if(sM.startsWith("h "))
    {
        sM=sM.mid(2);
        qDebug()<<"hack:"+sM;
        bool bCharged=false;
        QStringList asP=sM.split(",");
        forint(i,asP)
        {
            QString sP=asP[i];
            DataCfg::TPortal* pxP=DataCfg::Get()->m_xGameEnts.GetPortal(sP);
            if(!pxP) {continue;}
            if(pxP->GetChargePrc()<100&&(iMacroMode==0||iMacroMode==2)&&rxPlayer.m_eTeam==pxP->m_eTeam)
            {

                API::Get()->RPCRecharge(sP);
                if(pxP->GetChargePrc()<99) {bCharged=true;};
            };
            if(iMacroMode<=1)
            {
                API::Get()->RPCHack(sP);
            };
        };
        if(iMacroMode==2&&bCharged)
        {
            //retry
            m_iMacroIdx--;
            if(DataCfg::Get()->m_xPlayer.m_iXM<=1000) {OnBtnXM();};
            return;
        };
    };

    //OnBtnXM()
}

void InfluxWnd::AssignBtnShortcut(QWidget *p_pxBtn,QKeySequence p_sKey)
{
    QShortcut *pxSC=new QShortcut(p_sKey,this);
    connect(pxSC, SIGNAL(activated()),p_pxBtn, SIGNAL(clicked()));
}

bool InfluxWnd::event(QEvent* p_xEvt) {
    if(p_xEvt->type() == QEvent::LayoutRequest) {
        OnResize();
    }
    return QMainWindow::event(p_xEvt);
}

void InfluxWnd::on_actionLogin_triggered()
{
    //qDebug() << "Login";
    DlgLogin* pxDlg=new DlgLogin();
    connect(pxDlg,SIGNAL(LoginOk()),this,SLOT(OnUserLogin()));
}

void InfluxWnd::on_actionLogout_triggered()
{
    qDebug() << "Logout";
    DataCfg::Get()->Logout();
}

void InfluxWnd::on_actionLoad_User_triggered()
{
    qDebug() << "Load";
}

void InfluxWnd::on_actionSave_User_triggered()
{
    qDebug() << "Save";
    DataCfg::Get()->SaveSnapshot();
    UpdateUsers();
}

void InfluxWnd::UpdateUsers()
{
    QStringList asS=DataCfg::Get()->GetSnapshots();
    m_pxMenuLoad->clear();
    foreach(QString s,asS)
    {
        connect(m_pxMenuLoad->addAction(s),SIGNAL(triggered()),this,SLOT(OnLoadUser()));
    };
}

void InfluxWnd::OnLoadUser()
{
    QString sName=((QAction*)sender())->text();
    qDebug() << "Load:" << sName;
    if(!DataCfg::Get()->LoadSnapshot(sName)) {return;};
    OnUserLogin();
}

void InfluxWnd::OnMapMoved()
{
    m_iZoom=m_pxMapView->zoomLevel();
    m_xPos=m_pxMapView->center();
    ui->edtPos->setText(QString("?ll=%1,%2&z=%3").arg(QString::number(m_xPos.y(),'f',5)).arg(QString::number(m_xPos.x(),'f',5)).arg(m_iZoom));
    m_pxPlayer->setPos(m_xPos);
    API::Get()->m_xPos=m_xPos;
    DataCfg::Get()->m_xUserCfg.m_xPos=m_xPos;
    DataCfg::Get()->m_xUserCfg.m_iCurZ=m_iZoom;
    //qDebug() << m_pxMapView->zoomLevel() << m_pxMapView->mapToScene(QPoint(m_pxMapView->width()/2,m_pxMapView->height()/2));
    OnUpdSpeed();
}

void InfluxWnd::OnBtnInv()
{
    qDebug() << "Inv...";
    API::Get()->RPCInventory();
}

void InfluxWnd::OnBtnInvClear()
{
    qDebug() << "InvClear...";
    DataCfg::Get()->m_xInventory.Clear();
    DataCfg::Get()->m_xUserCfg.m_sLastInvSync="0";
    emit OnUpdInventory();
}

void InfluxWnd::OnBtnScn()
{
    qDebug() << "Scn...";
    API::Get()->RPCScan();
}

void InfluxWnd::OnBtnXM()
{
    qDebug() << "XM...";
    //qDebug() <<  QApplication::mouseButtons();
    API::Get()->RPCXM();
}

void InfluxWnd::OnBtnXMFlush()
{
    if(QMessageBox::Yes!=QMessageBox::question(this,tr("Confirm"),tr("Flush XM cache?"))) {return;};
    DataCfg::TEnergy& rxEC=DataCfg::Get()->m_xInfluxData.m_xEnergyCache;
    qDebug() << "XMFlush:" << rxEC.EnergySum() << rxEC.m_asGlobs.size();
    rxEC.Clear();
    OnUpdPortals();
}

void InfluxWnd::OnBtnXMSlurp()
{
    qDebug() << "XMSlurp...";
    API::Get()->RPCXMSlurp();
}

void InfluxWnd::OnBtnLinkQuery()
{
    QString sPortal=GetSelectedPortalGUID();
    if(sPortal.isEmpty()) {qDebug() << "Select a portal!";return;};

    QStringList asPortals;
    foreach(CircleObject* p,m_axPortals)
    {
        if(sPortal!=p->m_sGUID&&!asPortals.contains(p->m_sGUID))
        {
            asPortals.append(p->m_sGUID);
        };
    };
    if(asPortals.count()<=0) {qDebug() << "No Portals.";return;};

    QStringList asKeys=DataCfg::Get()->m_xInventory.FetchKeysByPortal(asPortals);
    if(asKeys.count()<=0) {qDebug() << "No Keys.";return;};

    qDebug() << "LinkQuery..." << asKeys.count();
    //qDebug() <<asKeys;return;
    //API::Get()->RPCLinkQuery(sPortal,asKeys);

    ui->lstLink->clear();
    foreach(QString s,asKeys)
    {
        DataCfg::TItem* pxI=DataCfg::Get()->m_xInventory.GetItem(s);
        if(!pxI) {continue;};
        ui->lstLink->addItem(pxI->m_sKeyPortalName);
        ui->lstLink->item(ui->lstLink->count()-1)->setData(Qt::UserRole,s);
    };
}

void InfluxWnd::OnBtnLinkBuild()
{
    QString sPortal=GetSelectedPortalGUID();
    if(sPortal.isEmpty()) {qDebug() << "Select a portal!";return;};
    if(!ui->lstLink->currentItem()) {qDebug() << "Select a key!";return;};
    QString sKey=ui->lstLink->currentItem()->data(Qt::UserRole).toString();
    DataCfg::TItem* pxI=DataCfg::Get()->m_xInventory.GetItem(sKey);
    if(!pxI) {qDebug() << "Select a key!";return;};

    qDebug() << "LinkBuild..." << sPortal<<sKey;
    API::Get()->RPCLinkBuild(sPortal,pxI->m_sKeyPortalGUID,sKey);
}

void InfluxWnd::OnBtnItem()
{
    bool ok;
    QString s=QInputDialog::getText(this,tr("Item Action"),tr("Cmd: d|r|u cnt  (drop,recycle,use)"),QLineEdit::Normal,"");
    QStringList as=s.split(" ");
    if(as.count()<2) {return;};
    QString sCmd=as[0].toLower();
    int iCnt=as[1].toInt();
    if(iCnt<=0) {return;};

    QObject* pxS=sender();
    int iLevel=-1;
    DataCfg::ItemType eType=DataCfg::IT_Invalid;

    if(ui->btnIKEY==pxS) {iLevel=0;eType=DataCfg::IT_Key;};
    if(ui->btnIC1==pxS) {iLevel=1;eType=DataCfg::IT_Cube;};
    if(ui->btnIC2==pxS) {iLevel=2;eType=DataCfg::IT_Cube;};
    if(ui->btnIC3==pxS) {iLevel=3;eType=DataCfg::IT_Cube;};
    if(ui->btnIC4==pxS) {iLevel=4;eType=DataCfg::IT_Cube;};
    if(ui->btnIC5==pxS) {iLevel=5;eType=DataCfg::IT_Cube;};
    if(ui->btnIC6==pxS) {iLevel=6;eType=DataCfg::IT_Cube;};
    if(ui->btnIC7==pxS) {iLevel=7;eType=DataCfg::IT_Cube;};
    if(ui->btnIC8==pxS) {iLevel=8;eType=DataCfg::IT_Cube;};
    if(ui->btnIX1==pxS) {iLevel=1;eType=DataCfg::IT_XMP;};
    if(ui->btnIX2==pxS) {iLevel=2;eType=DataCfg::IT_XMP;};
    if(ui->btnIX3==pxS) {iLevel=3;eType=DataCfg::IT_XMP;};
    if(ui->btnIX4==pxS) {iLevel=4;eType=DataCfg::IT_XMP;};
    if(ui->btnIX5==pxS) {iLevel=5;eType=DataCfg::IT_XMP;};
    if(ui->btnIX6==pxS) {iLevel=6;eType=DataCfg::IT_XMP;};
    if(ui->btnIX7==pxS) {iLevel=7;eType=DataCfg::IT_XMP;};
    if(ui->btnIX8==pxS) {iLevel=8;eType=DataCfg::IT_XMP;};
    if(ui->btnIR1==pxS) {iLevel=1;eType=DataCfg::IT_Resonator;};
    if(ui->btnIR2==pxS) {iLevel=2;eType=DataCfg::IT_Resonator;};
    if(ui->btnIR3==pxS) {iLevel=3;eType=DataCfg::IT_Resonator;};
    if(ui->btnIR4==pxS) {iLevel=4;eType=DataCfg::IT_Resonator;};
    if(ui->btnIR5==pxS) {iLevel=5;eType=DataCfg::IT_Resonator;};
    if(ui->btnIR6==pxS) {iLevel=6;eType=DataCfg::IT_Resonator;};
    if(ui->btnIR7==pxS) {iLevel=7;eType=DataCfg::IT_Resonator;};
    if(ui->btnIR8==pxS) {iLevel=8;eType=DataCfg::IT_Resonator;};
    if(ui->btnIS1==pxS) {iLevel=1;eType=DataCfg::IT_Shield;};
    if(ui->btnIS2==pxS) {iLevel=2;eType=DataCfg::IT_Shield;};
    if(ui->btnIS3==pxS) {iLevel=3;eType=DataCfg::IT_Shield;};
    if(ui->btnIL1==pxS) {iLevel=1;eType=DataCfg::IT_LinkAmp;};
    if(ui->btnIL2==pxS) {iLevel=2;eType=DataCfg::IT_LinkAmp;};
    if(ui->btnIL3==pxS) {iLevel=3;eType=DataCfg::IT_LinkAmp;};
    if(ui->btnIU1==pxS) {iLevel=1;eType=DataCfg::IT_Multihack;};
    if(ui->btnIU2==pxS) {iLevel=2;eType=DataCfg::IT_Multihack;};
    if(ui->btnIU3==pxS) {iLevel=3;eType=DataCfg::IT_Multihack;};
    if(ui->btnIH1==pxS) {iLevel=1;eType=DataCfg::IT_HeatSink;};
    if(ui->btnIH2==pxS) {iLevel=2;eType=DataCfg::IT_HeatSink;};
    if(ui->btnIH3==pxS) {iLevel=3;eType=DataCfg::IT_HeatSink;};
    if(ui->btnIF1==pxS) {iLevel=1;eType=DataCfg::IT_ForceAmp;};
    if(ui->btnIF2==pxS) {iLevel=2;eType=DataCfg::IT_ForceAmp;};
    if(ui->btnIF3==pxS) {iLevel=3;eType=DataCfg::IT_ForceAmp;};
    if(ui->btnIT1==pxS) {iLevel=1;eType=DataCfg::IT_Turret;};
    if(ui->btnIT2==pxS) {iLevel=2;eType=DataCfg::IT_Turret;};
    if(ui->btnIT3==pxS) {iLevel=3;eType=DataCfg::IT_Turret;};
    if(ui->btnIMED==pxS) {iLevel=0;eType=DataCfg::IT_Media;};
    if(ui->btnIADA==pxS) {iLevel=0;eType=DataCfg::IT_Ada;};
    if(ui->btnIJAR==pxS) {iLevel=0;eType=DataCfg::IT_Jarvis;};
    if(eType==DataCfg::IT_Invalid) {return;};

    QStringList asItems=DataCfg::Get()->m_xInventory.Fetch(eType,iCnt,iLevel);
    if(asItems.count()<=0) {qDebug()<<"not items";return;};

    if(sCmd=="d")
    {
        qDebug() << "Drop Items..."<<asItems.count();
        for(int i=0;i<asItems.count();i++)
        {
            API::Get()->RPCDrop(asItems[i]);
        };
        return;
    };
    if(sCmd=="r")
    {
        qDebug() << "Recycle Items..."<<asItems.count();
        for(int i=0;i<asItems.count();i++)
        {
            API::Get()->RPCRecycle(asItems[i]);
        };
        return;
    };
    if(sCmd=="u")
    {
        if(eType==DataCfg::IT_Cube)
        {
            qDebug() << "Use Cube ..."<<asItems.count();
            API::Get()->RPCUseCube(asItems[0]);
        }
        else if(eType==DataCfg::IT_XMP)
        {
            qDebug() << "Fire XMP ..."<<asItems.count();
            m_asXMPQ+=asItems;
//            for(int i=0;i<asItems.count();i++)
//            {
//                API::Get()->RPCFireXMP(asItems[i]);
//            };
        }
        else if(eType==DataCfg::IT_Ada||eType==DataCfg::IT_Jarvis)
        {
            QString sPortal=GetSelectedPortalGUID();
            if(sPortal.isEmpty()) {qDebug() << "Select a portal!";return;};

            qDebug() << "Flip Portal ..."<<asItems.count();
            API::Get()->RPCFlip(sPortal,asItems[0]);
        }
        else if(eType>=DataCfg::IT_Shield)
        {
            QString sPortal=GetSelectedPortalGUID();
            if(sPortal.isEmpty()) {qDebug() << "Select a portal!";return;};
            DataCfg::TPortal* pxP=DataCfg::Get()->m_xGameEnts.GetPortal(sPortal);
            if(!pxP) {qDebug() << "Select a portal!";return;};
            int iSlot=pxP->GetUpgradeModSlot();
            if(iSlot==-1) {qDebug() << "No free Slot!";return;};

            qDebug() << "Add Mod ..."<<asItems.count();
            API::Get()->RPCAddMod(sPortal,asItems[0],iSlot);
        }
        else
        {
            qWarning() << "Can't use item";
        };
        //for(int i=0;i<asItems.count();i++)
        //{
        //    API::Get()->RPCUseCube(asItems[i]);
        //};
        return;
    };

    qDebug() << "Item..." << as;
}

void InfluxWnd::OnBtnXMPLo()
{
    QStringList asItems;
    int iTry=1;
    while(iTry<=DataCfg::Get()->m_xPlayer.m_iLevel)
    {
        asItems=DataCfg::Get()->m_xInventory.Fetch(DataCfg::IT_XMP,100,iTry);
        if(asItems.size()>0) {break;};
        iTry++;
    };
    if(asItems.size()<=0) {qDebug() << "No XMP!";return;};
    API::Get()->RPCFireXMP(asItems[qrand()%asItems.size()]);
}

void InfluxWnd::OnBtnXMPHi()
{
    QStringList asItems;
    int iTry=DataCfg::Get()->m_xPlayer.m_iLevel;
    while(iTry>=1)
    {
        asItems=DataCfg::Get()->m_xInventory.Fetch(DataCfg::IT_XMP,100,iTry);
        if(asItems.size()>0) {break;};
        iTry--;
    };
    if(asItems.size()<=0) {qDebug() << "No XMP!";return;};
    API::Get()->RPCFireXMP(asItems[qrand()%asItems.size()]);
}

void InfluxWnd::OnBtnPickSel()
{
    QString sItem;
    for(int i=0;i<m_axItems.size();i++)
    {
        if(m_axItems[i]->isSelected())
        {
            sItem=m_axItems[i]->m_sGUID;
            break;
        }
    };
    if(sItem.isEmpty()) {qDebug() << "Select an item!";return;};

    qDebug() << "Pickup...";

    API::Get()->RPCPickup(sItem);
}

void InfluxWnd::OnBtnPickAll()
{
    QStringList asI=DataCfg::Get()->m_xGameEnts.FetchItems(m_xPos);
    for(int i=0;i<asI.size();i++)
    {
        API::Get()->RPCPickup(asI[i]);
    };
}


void InfluxWnd::OnBtnRecharge()
{
    QString sPortal=GetSelectedPortalGUID();
    if(sPortal.isEmpty())
    {
        //if(MacroIsRecording()) {MacroMoveHere();m_sMacroRec+="r *;";return;}
        QStringList asP=DataCfg::Get()->m_xGameEnts.FetchPortals(m_xPos);
        qDebug() << "No Portal selected, recharging:" << asP.size();
        for(int i=0;i<asP.size();i++)
        {
            API::Get()->RPCRecharge(asP[i]);
        };
        return;
    };

    qDebug() << "Recharge...";
    //if(MacroIsRecording()) {MacroMoveHere();m_sMacroRec+="r "+sPortal+";";return;}
    API::Get()->RPCRecharge(sPortal);
}

void InfluxWnd::OnBtnHack()
{
    int iCount=((QPushButton*)sender())->text().split(" ").last().toInt();
    if(iCount<1) {return;};
    DoHack(iCount);
}

void InfluxWnd::OnBtnDeployLo() {DoDeploy(true);}
void InfluxWnd::OnBtnDeployHi() {DoDeploy(false);}

QString InfluxWnd::GetSelectedPortalGUID()
{
    QString sPortal;
    for(int i=0;i<m_axPortals.size();i++)
    {
        if(m_axPortals[i]->isSelected())
        {
            sPortal=m_axPortals[i]->m_sGUID;
            break;
        }
    };
    return sPortal;
}

void InfluxWnd::DoHack(int p_iCount)
{
    if(p_iCount<1) {p_iCount=1;}
    QString sPortal=GetSelectedPortalGUID();
    if(sPortal.isEmpty()&&p_iCount>1) {qDebug() << "Select a portal!";return;};

    if(sPortal.isEmpty())
    {
        QStringList asP=DataCfg::Get()->m_xGameEnts.FetchPortals(m_xPos);
        if(MacroIsRecording()) {MacroMoveHere();m_sMacroRec+="h "+asP.join(',')+";";qDebug()<<"h "+QString::number(asP.size());return;}

        qDebug() << "No Portal selected, hacking:" << asP.size();
        for(int i=0;i<asP.size();i++)
        {
            API::Get()->RPCHack(asP[i]);
        };
        return;
    };

    if(MacroIsRecording()) {MacroMoveHere();m_sMacroRec+="h "+sPortal+" "+QString::number(p_iCount)+";";qDebug()<<"h "<<sPortal;return;}

    qDebug() << "Hack...";
    //disable multihack!!!
    p_iCount=1;
    for(int i=0;i<p_iCount;i++)
    {
        API::Get()->RPCHack(sPortal);
    };
}

void InfluxWnd::DoDeploy(bool p_bLow)
{
    QString sPortal=GetSelectedPortalGUID();
    if(sPortal.isEmpty()) {qDebug() << "Select a portal!";return;};
    DataCfg::TPortal* pxP=DataCfg::Get()->m_xGameEnts.GetPortal(sPortal);
    if(!pxP) {return;}

    bool bUpgrade=false;
    int iSlot=0;
    QStringList asResos=PortalGetUpgradableReso(*pxP,bUpgrade,iSlot,p_bLow);
    if(asResos.count()<1) {qDebug() << "Missing Reso...";return;};

    if(bUpgrade)
    {
        qDebug() << "Upgrade...";
        API::Get()->RPCUpgrade(sPortal,asResos[qrand()%asResos.count()],iSlot);
    }
    else
    {
        qDebug() << "Deploy...";
        API::Get()->RPCDeploy(sPortal,asResos[qrand()%asResos.count()],iSlot);
    };
}

QStringList InfluxWnd::PortalGetUpgradableReso(DataCfg::TPortal& p_xPortal,bool& po_bUpgrade,int& po_iSlot,bool p_bLow)
{
    QStringList asResos;
    DataCfg::TPortal* pxP=&p_xPortal;
    if(!pxP) {return asResos;}
    DataCfg::TPlayer& rxPlayer=DataCfg::Get()->m_xPlayer;
    if(rxPlayer.m_eTeam!=pxP->m_eTeam&&pxP->m_eTeam!=DataCfg::T_Neutral) {return asResos;};

    QString sPlayer=rxPlayer.m_sGUID;
    po_bUpgrade=false;
    po_iSlot=0;
    int iMax=DataCfg::Get()->m_xPlayer.m_iLevel;
    int iCur=p_bLow?1:iMax;

    if(pxP->m_axResonators.count()>1)
    {
        //randomize deploy
        if(qrand()%100<15) {po_iSlot=qrand()%7;};
    };

    while(true)
    {
        int iFetch=iCur;
        if(p_bLow) {iCur++;} else {iCur--;};
        if(iFetch<1||iFetch>iMax) {break;};
        if(!pxP->CanDeployResonator(sPlayer,iFetch)) {continue;};
        asResos=DataCfg::Get()->m_xInventory.Fetch(DataCfg::IT_Resonator,1000,iFetch);
        if(asResos.count()>0) {break;};
    };
    if(pxP->m_axResonators.count()>=8)
    {
        //full -> upgrade
        int iSlot=0;
        iCur=p_bLow?1:iMax;
        while(true)
        {
            int iFetch=iCur;
            if(p_bLow) {iCur++;} else {iCur--;};
            if(iFetch<1||iFetch>iMax) {break;};
            iSlot=pxP->GetUpgradeResonatorSlot(sPlayer,iFetch);
            if(iSlot==-1) {continue;};
            asResos=DataCfg::Get()->m_xInventory.Fetch(DataCfg::IT_Resonator,1000,iFetch);
            if(asResos.count()>0) {break;};
        };
        if(asResos.count()>0)
        {
            po_bUpgrade=true;
            po_iSlot=iSlot;
        };
    };
    return asResos;
}

void InfluxWnd::OnUpdPortals()
{
    m_xTimerUpdatePortals.stop();
    m_xTimerUpdatePortals.start(200);
}

void InfluxWnd::OnUpdPortals_Defer()
{
    ui->btnXM->setText("XM:"+QString::number(DataCfg::Get()->m_xEnergy.EnergySum()));
    DataCfg::TEnergy& rxEC=DataCfg::Get()->m_xInfluxData.m_xEnergyCache;
    ui->btnXMFlush->setEnabled(rxEC.m_asGlobs.size()>0);

    MapGraphicsObject* pxSel=NULL;

    QStringList& axE=DataCfg::Get()->m_xEnergy.m_asGlobs;
    static QStringList asCacheE;
    if(asCacheE!=axE)
    {
        asCacheE=axE;
        for(int i=0;i<m_axEnergy.size();i++) {delete m_axEnergy[i];};m_axEnergy.clear();
        for(int i=0;i<axE.size();i++)
        {
            QString sE=axE[i];
            QPointF vPos=DataCfg::TEnergy::GlobPos(sE);
            float fVal=DataCfg::TEnergy::GlobValue(sE);
            //qDebug() << fVal;

            CircleObject* pxP=new CircleObject(1+fVal/100,false,QColor(255,255,255,255),NULL);
            //pxP->SetPortal(rxP.m_sGUID,rxP,asKeys.count());
            pxP->m_bGlob=true;
            m_pxMapScene->addObject(pxP);
            pxP->setPos(vPos);
            m_axEnergy.append(pxP);
        };
    };

    QList<DataCfg::TField>& axF=DataCfg::Get()->m_xGameEnts.m_axFields;
    for(int i=0;i<m_axFields.size();i++) {delete m_axFields[i];};m_axFields.clear();
    for(int i=0;i<axF.size();i++)
    {
        DataCfg::TField& rxF=axF[i];
        QPolygonF poly;
        poly.append(rxF.m_vA);
        poly.append(rxF.m_vB);
        poly.append(rxF.m_vC);

        QColor col=(rxF.m_eTeam==DataCfg::T_Aliens)?QColor(20,250,20,50):QColor(100,100,250,50);
        PolygonObject* pxL=new PolygonObject(poly,col);
        m_pxMapScene->addObject(pxL);
        m_axFields.append(pxL);
        //break;
    };

    QList<DataCfg::TPortal>& axP=DataCfg::Get()->m_xGameEnts.m_axPortals;
    for(int i=0;i<m_axPortals.size();i++) {delete m_axPortals[i];};m_axPortals.clear();
    for(int i=0;i<m_axLinks.size();i++) {delete m_axLinks[i];};m_axLinks.clear();
    for(int i=0;i<axP.size();i++)
    {
        //if(m_axLinks.size()>0) break;

        DataCfg::TPortal& rxP=axP[i];
        for(int iE=0;iE<rxP.m_asEdges.size();iE++)
        {
            QString sE=rxP.m_asEdges[iE];
            for(int j=0;j<axP.size();j++)
            {
                if(axP[j].m_sGUID!=sE) {continue;};
                LineObject* pxL=new LineObject(rxP.m_xPos,axP[j].m_xPos,0.5);
                m_pxMapScene->addObject(pxL);
                m_axLinks.append(pxL);
                //qDebug() << rxP.m_sName << axP[j].m_sName;
                break;
            };
        };
    };


    for(int i=0;i<axP.size();i++)
    {
        DataCfg::TPortal& rxP=axP[i];
        QStringList asKeys=DataCfg::Get()->m_xInventory.FetchKeysByPortal(QStringList()<<rxP.m_sGUID);
        bool bUpg=false;
        int iSl=0;
        QStringList asResos=PortalGetUpgradableReso(rxP,bUpg,iSl,false);
        //if(!bUpg) {asResos.clear();};

        QColor col=(rxP.m_eTeam==DataCfg::T_Aliens)?QColor(20,250,20):(rxP.m_eTeam==DataCfg::T_Humans)?QColor(100,100,250):QColor(180,180,180);
        CircleObject* pxP=new CircleObject(6,false,col,NULL);
        pxP->SetPortal(rxP.m_sGUID,rxP,asKeys.count(),asResos.count());
        m_pxMapScene->addObject(pxP);
        pxP->setPos(rxP.m_xPos);
        m_axPortals.append(pxP);
        connect(pxP,SIGNAL(selectedChanged()),this,SLOT(OnPortalSelect()));

        if(rxP.m_sGUID==m_sCurSel) {pxSel=pxP;};
    };

    QList<DataCfg::TItem>& axI=DataCfg::Get()->m_xGameEnts.m_axItems;
    for(int i=0;i<m_axItems.size();i++) {delete m_axItems[i];};m_axItems.clear();
    qDebug() << "Items:" << axI.count();
    for(int i=0;i<axI.size();i++)
    {
        DataCfg::TItem& rxI=axI[i];

        QColor col=QColor(240,240,20);
        CircleObject* pxP=new CircleObject(4,false,col,NULL);
        QString sI="";
        if(rxI.m_eType==DataCfg::IT_Ada) {sI="A";};
        if(rxI.m_eType==DataCfg::IT_Jarvis) {sI="J";};
        if(rxI.m_eType==DataCfg::IT_Cube) {sI="C";};
        if(rxI.m_eType==DataCfg::IT_Key) {sI="K";};
        if(rxI.m_eType==DataCfg::IT_Resonator) {sI="R";};
        if(rxI.m_eType==DataCfg::IT_Shield) {sI="S";};
        if(rxI.m_eType==DataCfg::IT_XMP) {sI="X";};
        if(rxI.m_eType==DataCfg::IT_Media) {sI="M";};

        if(rxI.m_eType==DataCfg::IT_LinkAmp) {sI="L";};
        if(rxI.m_eType==DataCfg::IT_Multihack) {sI="U";};
        if(rxI.m_eType==DataCfg::IT_HeatSink) {sI="H";};
        if(rxI.m_eType==DataCfg::IT_ForceAmp) {sI="F";};
        if(rxI.m_eType==DataCfg::IT_Turret) {sI="T";};

        sI+=QString::number(rxI.m_iLevel);

        pxP->SetItem(rxI.m_sGUID,sI);
        m_pxMapScene->addObject(pxP);
        pxP->setPos(rxI.m_xPos);
        m_axItems.append(pxP);
    };

    if(pxSel)
    {
        pxSel->setSelected(true);
        //qDebug() << "Resel:" << pxSel;
    };
}

void InfluxWnd::OnPortalSelect()
{
    QString sPortal=GetSelectedPortalGUID();
    DataCfg::TPortal* pxP=DataCfg::Get()->m_xGameEnts.GetPortal(sPortal);
    m_sCurSel=sPortal;
    ui->lblPName->setText(pxP?pxP->m_sName:"<none>");
}

void InfluxWnd::OnUpdInventory(QStringList p_asNew)
{
    //qDebug() << ui->tblItems->item(0,0);
    DataCfg::TInventory& rxInv=DataCfg::Get()->m_xInventory;
    if(rxInv.m_aiCountR.count()<8) {return;};

    ui->btnIR1->setText(QString::number(rxInv.m_aiCountR[0]));
    ui->btnIR2->setText(QString::number(rxInv.m_aiCountR[1]));
    ui->btnIR3->setText(QString::number(rxInv.m_aiCountR[2]));
    ui->btnIR4->setText(QString::number(rxInv.m_aiCountR[3]));
    ui->btnIR5->setText(QString::number(rxInv.m_aiCountR[4]));
    ui->btnIR6->setText(QString::number(rxInv.m_aiCountR[5]));
    ui->btnIR7->setText(QString::number(rxInv.m_aiCountR[6]));
    ui->btnIR8->setText(QString::number(rxInv.m_aiCountR[7]));

    ui->btnIX1->setText(QString::number(rxInv.m_aiCountX[0]));
    ui->btnIX2->setText(QString::number(rxInv.m_aiCountX[1]));
    ui->btnIX3->setText(QString::number(rxInv.m_aiCountX[2]));
    ui->btnIX4->setText(QString::number(rxInv.m_aiCountX[3]));
    ui->btnIX5->setText(QString::number(rxInv.m_aiCountX[4]));
    ui->btnIX6->setText(QString::number(rxInv.m_aiCountX[5]));
    ui->btnIX7->setText(QString::number(rxInv.m_aiCountX[6]));
    ui->btnIX8->setText(QString::number(rxInv.m_aiCountX[7]));

    ui->btnIC1->setText(QString::number(rxInv.m_aiCountC[0]));
    ui->btnIC2->setText(QString::number(rxInv.m_aiCountC[1]));
    ui->btnIC3->setText(QString::number(rxInv.m_aiCountC[2]));
    ui->btnIC4->setText(QString::number(rxInv.m_aiCountC[3]));
    ui->btnIC5->setText(QString::number(rxInv.m_aiCountC[4]));
    ui->btnIC6->setText(QString::number(rxInv.m_aiCountC[5]));
    ui->btnIC7->setText(QString::number(rxInv.m_aiCountC[6]));
    ui->btnIC8->setText(QString::number(rxInv.m_aiCountC[7]));

    ui->btnIS1->setText(QString::number(rxInv.m_aiCountS[0]));
    ui->btnIS2->setText(QString::number(rxInv.m_aiCountS[1]));
    ui->btnIS3->setText(QString::number(rxInv.m_aiCountS[2]));
    ui->btnIL1->setText(QString::number(rxInv.m_aiCountL[0]));
    ui->btnIL2->setText(QString::number(rxInv.m_aiCountL[1]));
    ui->btnIL3->setText(QString::number(rxInv.m_aiCountL[2]));
    ui->btnIU1->setText(QString::number(rxInv.m_aiCountU[0]));
    ui->btnIU2->setText(QString::number(rxInv.m_aiCountU[1]));
    ui->btnIU3->setText(QString::number(rxInv.m_aiCountU[2]));
    ui->btnIH1->setText(QString::number(rxInv.m_aiCountH[0]));
    ui->btnIH2->setText(QString::number(rxInv.m_aiCountH[1]));
    ui->btnIH3->setText(QString::number(rxInv.m_aiCountH[2]));
    ui->btnIF1->setText(QString::number(rxInv.m_aiCountF[0]));
    ui->btnIF2->setText(QString::number(rxInv.m_aiCountF[1]));
    ui->btnIF3->setText(QString::number(rxInv.m_aiCountF[2]));
    ui->btnIT1->setText(QString::number(rxInv.m_aiCountT[0]));
    ui->btnIT2->setText(QString::number(rxInv.m_aiCountT[1]));
    ui->btnIT3->setText(QString::number(rxInv.m_aiCountT[2]));

    ui->btnIAll->setText(QString::number(rxInv.m_axItems.size()));
    ui->btnIKEY->setText(QString::number(rxInv.m_iCountK));
    ui->btnIADA->setText(QString::number(rxInv.m_iCountA));
    ui->btnIJAR->setText(QString::number(rxInv.m_iCountJ));
    ui->btnIMED->setText(QString::number(rxInv.m_iCountM));

    if(p_asNew.size()<=0) {return;};
    if(!ui->chkAutoRec->isChecked()) {return;};
    QStringList asKeys=rxInv.Fetch(DataCfg::IT_Key,2000,0);
    forint(i,p_asNew)
    {
        if(!asKeys.contains(p_asNew[i])) {continue;};
        qDebug() << "Autorecycle:" << p_asNew[i];
        API::Get()->RPCRecycle(p_asNew[i]);
    };
}

void InfluxWnd::OnUpdPlayer()
{
    DataCfg::TPlayer& rxP=DataCfg::Get()->m_xPlayer;
    ui->lblName->setText(rxP.m_sNick);
    ui->lblAP->setText("AP:"+QString::number(rxP.m_iAP)+" L:"+QString::number(rxP.m_iLevel));
    ui->prgXM->setRange(0,rxP.m_iXMMax);
    ui->prgXM->setValue(rxP.m_iXM);
}

void InfluxWnd::OnResize()
{
    int iW=ui->centralWidget->size().width();
    int iH=ui->centralWidget->size().height();
    if(iW<=0||iH<=0) {return;};

    QPointF xPos=m_xPos;
    bool b=m_pxMapView->blockSignals(true);

    //qDebug() << iW << iH << ui->centralWidget->size();
    ui->wdgTool->move(iW-ui->wdgTool->width()-4,ui->wdgTool->y());
    m_pxMapView->move(ui->wdgMeta->width(),m_pxMapView->y());
    m_pxMapView->resize(iW-ui->wdgTool->width()-ui->wdgMeta->width(),iH-m_pxMapView->y()-200);
    m_pxMapView->resetQGSSceneSize();

    m_pxLogger->move(ui->wdgMeta->width()+2,m_pxMapView->y()+m_pxMapView->height());
    m_pxLogger->resize(m_pxMapView->width()-20,200);
    m_pxLogger->OnResize();

    m_xPos=xPos;
    m_pxMapView->centerOn(xPos);
    //OnMapMoved();
    m_pxMapView->blockSignals(b);
}

void InfluxWnd::OnUserLogin()
{
    OnUpdPlayer();
    OnUpdInventory();
    OnUpdPortals();

    QPointF xPos=DataCfg::Get()->m_xUserCfg.m_xPos;
    MapGoTo(xPos,DataCfg::Get()->m_xUserCfg.m_iCurZ);
}

void InfluxWnd::OnEdtPos()
{
    DataCfg::TMapPos xPos=DataCfg::ParseURLPos(ui->edtPos->text());
    if(!xPos.IsValid()) {qError()<<"Invalid Position:"<<ui->edtPos->text();return;};
    MapGoTo(xPos.m_xLL,xPos.m_iZ);
}

void InfluxWnd::MapGoTo(QPointF p_xPos,int p_iZ)
{
    if(p_xPos.x()==0&&p_xPos.y()==0) {return;};
    if(p_iZ<=0) {return;};

    bool b=m_pxMapView->blockSignals(true);
    m_pxMapView->setZoomLevel(p_iZ);
    m_pxMapView->centerOn(p_xPos);
    OnMapMoved();
    m_pxMapView->blockSignals(b);
}

void InfluxWnd::UpdateConfig()
{
    int iXMK=0;
    ui->lstLocations->clear();
    QList<DataCfg::TInfluxCfg::TPlace>& axPlaces=DataCfg::Get()->m_xInfluxCfg.m_axPlaces;
    forint(i,axPlaces)
    {
        DataCfg::TInfluxCfg::TPlace& rxP=axPlaces[i];
        QString sLbl=rxP.m_sName;
        DataCfg::TInfluxData::TPlaceXM* pxPS=DataCfg::Get()->m_xInfluxData.GetPlace(rxP.m_sName);
        if(pxPS&&rxP.m_fSlurpPrio>0)
        {
            int iE=pxPS->m_xEnergy.EnergySum()/1000;
            iXMK+=iE;

            sLbl+=" "+QString::number(iE)+"k";
        };
        if(!rxP.m_sMacro.isEmpty())
        {
            sLbl+=" #"+QString::number(rxP.m_sMacro.split(";",QString::SkipEmptyParts).size()/2)+"";
        };

        ui->lstLocations->addItem(sLbl);
        QListWidgetItem* pxItem=ui->lstLocations->item(ui->lstLocations->count()-1);
        pxItem->setCheckState(rxP.m_fSlurpPrio>0?Qt::Checked:Qt::Unchecked);
        pxItem->setData(Qt::UserRole,rxP.m_sName);

    };
    ui->lstLocations->sortItems();
    ui->btnXMSlurp->setText("XM:"+QString::number(iXMK)+"k");
}

void InfluxWnd::OnLocationItemChanged(QListWidgetItem* p_pxItem)
{
    QString sName=p_pxItem->data(Qt::UserRole).toString();
    bool bChecked=p_pxItem->checkState()==Qt::Checked;
    DataCfg::TInfluxCfg::TPlace* pxP=DataCfg::Get()->m_xInfluxCfg.GetPlace(sName);
    if(!pxP) {return;};
    bool bEnabled=pxP->m_fSlurpPrio>0;
    if(bChecked==bEnabled) {return;};
    pxP->m_fSlurpPrio=-pxP->m_fSlurpPrio;
    //qDebug() << "Check:" << sName;
    //UpdateConfig();
}

void InfluxWnd::OnDblClickedLocation(QModelIndex p_xIdx)
{
    QListWidgetItem *pxItm=(QListWidgetItem*)p_xIdx.internalPointer();
    if(!pxItm) {return;}
    QString sName=pxItm->data(Qt::UserRole).toString();
    qDebug() << "GoTo:" << sName;

    DataCfg::TInfluxCfg::TPlace* pxP=DataCfg::Get()->m_xInfluxCfg.GetPlace(sName);
    if(!pxP) {qError()<<"Unknown Place:"<<sName;return;};
    DataCfg::TMapPos xPos=DataCfg::ParseURLPos(pxP->m_sPos);
    if(!xPos.IsValid()) {qError()<<"Invalid Place Position:"<<sName<<pxP->m_sPos;return;};
    MapGoTo(xPos.m_xLL,xPos.m_iZ);
}

void InfluxWnd::OnBtnUpdatePlaces()
{
    QList<DataCfg::TInfluxCfg::TPlace>& axPlaces=DataCfg::Get()->m_xInfluxCfg.m_axPlaces;
    forint(i,axPlaces)
    {
        DataCfg::TInfluxCfg::TPlace& rxP=axPlaces[i];
        if(rxP.m_fSlurpPrio<=0) {continue;};
        DataCfg::TMapPos xPos=DataCfg::ParseURLPos(rxP.m_sPos);
        if(!xPos.IsValid()) {continue;};

        API::Get()->RPCScan(xPos.m_xLL,QStringList(),rxP.m_sName);
    };
}

void InfluxWnd::OnBtnPlaceAdd()
{
    QString sName=QInputDialog::getText(this,tr("Add Place"),tr("Name:"),QLineEdit::Normal,"");
    if(sName.isEmpty()) {return;}
    QString sPos=ui->edtPos->text();
    DataCfg::Get()->m_xInfluxCfg.AddPlace(sName,sPos,1);
    UpdateConfig();
}

void InfluxWnd::OnBtnPlaceRem()
{
    QList<QListWidgetItem*> axSel=ui->lstLocations->selectedItems();
    if(axSel.size()<=0) {return;}
    if(QMessageBox::Yes!=QMessageBox::question(this,tr("Delete"),tr("Selected Places?"))) {return;}
    foreach(QListWidgetItem* pxItm,axSel)
    {
        QString sName=pxItm->data(Qt::UserRole).toString();
        DataCfg::Get()->m_xInfluxCfg.RemPlace(sName);
        DataCfg::Get()->m_xInfluxData.RemPlace(sName);
    };
    UpdateConfig();
}

void InfluxWnd::OnBtnRec()
{
    QList<QListWidgetItem*> apxSel=ui->lstLocations->selectedItems();
    if(apxSel.size()<=0) {qError()<<"Select Place!";return;};
    QString sPlace=apxSel[0]->data(Qt::UserRole).toString();

    if(!MacroIsRecording())
    {
        ui->btnRec->setText("STOP");
        ui->lstLocations->setEnabled(false);
        m_sMacroPlace=sPlace;
        m_sMacroRec.clear();
        qDebug() << "Rec:" << sPlace;
    }
    else
    {
        if(QMessageBox::Yes==QMessageBox::question(this,tr("Save Macro?"),tr("Save Macro to:")+sPlace+" "+QString::number(m_sMacroRec.split(";",QString::SkipEmptyParts).count()/2)))
        {
            DataCfg::Get()->m_xInfluxCfg.SetPlaceMacro(sPlace,m_sMacroRec);
            UpdateConfig();
        };

        m_sMacroPlace.clear();
        ui->btnRec->setText("REC");
        ui->lstLocations->setEnabled(true);
        qDebug() << "Stop:" << sPlace;
    };

}

void InfluxWnd::OnBtnPlay()
{
    QList<QListWidgetItem*> apxSel=ui->lstLocations->selectedItems();
    if(apxSel.size()<=0) {qError()<<"Select Place!";return;};
    QString sPlace=apxSel[0]->data(Qt::UserRole).toString();
    if(!MacroIsPlaying())
    {
        qDebug() << "Play:" << sPlace;
        m_asMacroPlay=DataCfg::Get()->m_xInfluxCfg.GetPlaceMacro(sPlace).split(";",QString::SkipEmptyParts);
        if(m_asMacroPlay.isEmpty()) {return;}

        ui->btnPlay->setText("STOP");
        ui->lstLocations->setEnabled(false);
        m_iMacroIdx=0;

        ui->prgMacro->setEnabled(true);
        ui->prgMacro->setMaximum(m_asMacroPlay.size()/2);
        ui->prgMacro->setValue(0);
    }
    else
    {
        qDebug() << "Stop:" << sPlace;
        ui->btnPlay->setText("PLAY");
        ui->lstLocations->setEnabled(true);
        m_asMacroPlay.clear();
        ui->prgMacro->setEnabled(false);
        ui->prgMacro->setMaximum(1);
        ui->prgMacro->setValue(0);
    };

}
