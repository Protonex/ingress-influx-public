#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "datacfg.h"

namespace Ui {
class MainWindow;
}

class MapGraphicsView;
class MapGraphicsScene;
class MapGraphicsObject;
class CircleObject;
class LineObject;
class PolygonObject;
class LoggerWnd;

class InfluxWnd : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit InfluxWnd(QWidget *parent = 0);
    ~InfluxWnd();

    static InfluxWnd* ms_pxInst;
    QString m_sPathUser;
    QMenu* m_pxMenuLoad;
    LoggerWnd* m_pxLogger;

    int m_iZoom;
    QPointF m_xPos;

    QString m_sCurSel;
    QString m_sMacroRec;
    QString m_sMacroPlace;
    int m_iMacroIdx;
    QStringList m_asMacroPlay;
    QStringList m_asXMPQ;

    MapGraphicsView* m_pxMapView;
    MapGraphicsScene* m_pxMapScene;
    CircleObject* m_pxPlayer;
    QLabel* m_pxLblSpeed;
    LineObject* m_pxLineAction;
    QTimer m_xTimerUpdatePortals;
    QTimer m_xTimerUpdateSpeed;

    QList<CircleObject*> m_axPortals;
    QList<CircleObject*> m_axItems;
    QList<CircleObject*> m_axEnergy;
    QList<LineObject*> m_axLinks;
    QList<PolygonObject*> m_axFields;

    bool event(QEvent *p_xEvt);
    QString GetSelectedPortalGUID();
    void UpdateUsers();
    void MapGoTo(QPointF p_xPos, int p_iZ);
    QStringList PortalGetUpgradableReso(DataCfg::TPortal& p_xPortal,bool& po_bUpgrade,int& po_iSlot,bool p_bLow);
    void AssignBtnShortcut(QWidget *p_pxBtn,QKeySequence p_sKey);

    bool MacroIsRecording() {return !m_sMacroPlace.isEmpty();}
    bool MacroIsPlaying() {return !m_asMacroPlay.isEmpty();}
    void MacroMoveHere() {MacroMoveTo(m_xPos);}
    void MacroMoveTo(QPointF p_vPos) {m_sMacroRec+="m "+QString::number(p_vPos.y(),'f',6)+","+QString::number(p_vPos.x(),'f',6)+";";}

private slots:
    void OnUserLogin();
    void OnResize();
    void OnEdtPos();
    void OnUpdSpeed();
    void on_actionLogin_triggered();
    void on_actionLogout_triggered();
    void on_actionLoad_User_triggered();
    void on_actionSave_User_triggered();
    void OnMapMoved();
    void OnPortalSelect();
    void DoHack(int p_iCount);
    void DoDeploy(bool p_bLow);

    void OnBtnXMPLo();
    void OnBtnXMPHi();
    void OnBtnPickAll();
    void OnBtnInvClear();
    void OnBtnInv();
    void OnBtnScn();
    void OnBtnPickSel();
    void OnBtnRecharge();
    void OnBtnHack();
    void OnBtnXM();
    void OnBtnXMSlurp();
    void OnBtnXMFlush();
    void OnBtnDeployLo();
    void OnBtnDeployHi();
    void OnBtnItem();
    void OnBtnLinkQuery();
    void OnBtnLinkBuild();
    void OnBtnRec();
    void OnBtnPlay();

    void OnLoadUser();


    void OnUpdInventory(QStringList p_asNew=QStringList());
    void OnUpdPortals();
    void OnUpdPortals_Defer();
    void OnUpdPlayer();

    void UpdateConfig();
    void OnBtnUpdatePlaces();
    void OnLocationItemChanged(QListWidgetItem *p_pxItem);
    void OnDblClickedLocation(QModelIndex p_xIdx);
    void OnBtnPlaceAdd();
    void OnBtnPlaceRem();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
