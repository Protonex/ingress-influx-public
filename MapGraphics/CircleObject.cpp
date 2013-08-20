#include "CircleObject.h"
#include "s2.h"
#include <QtDebug>
#include <QKeyEvent>

CircleObject::CircleObject(qreal radius,bool sizeIsZoomInvariant, QColor fillColor, MapGraphicsObject *parent) :
    MapGraphicsObject(sizeIsZoomInvariant,parent), _fillColor(fillColor)
{
    _radius = qMax<qreal>(radius,0.01);
    m_bPlayer=false;
    m_bGlob=false;
    m_iKeyCnt=0;
    m_iResoCnt=0;

    //this->setFlag(MapGraphicsObject::ObjectIsSelectable);
    //this->setFlag(MapGraphicsObject::ObjectIsMovable);
    //this->setFlag(MapGraphicsObject::ObjectIsFocusable);
}

CircleObject::~CircleObject()
{
}

QRectF CircleObject::boundingRect() const
{
    int iAddX=0;
    int iAddY=0;
    if(m_xPortal.m_eTeam!=DataCfg::T_Invalid)
    {
        iAddX=4;
        iAddY=6;
    };

    return QRectF(-1*_radius-iAddX,
                  -1*_radius-iAddY,
                  2*_radius+iAddX*2,
                  2*_radius+iAddY*2);
}

void CircleObject::SetPortal(QString p_sGUID,DataCfg::TPortal p_xP,int p_iKeyCnt,int p_iResoCnt)
{
    this->setFlag(MapGraphicsObject::ObjectIsSelectable);
    this->setFlag(MapGraphicsObject::ObjectIsFocusable);

    m_sGUID=p_sGUID;
    m_xPortal=p_xP;
    m_iKeyCnt=p_iKeyCnt;
    m_iResoCnt=p_iResoCnt;
}

void CircleObject::SetItem(QString p_sGUID, QString p_sType)
{
    this->setFlag(MapGraphicsObject::ObjectIsSelectable);
    this->setFlag(MapGraphicsObject::ObjectIsFocusable);

    m_sGUID=p_sGUID;
    m_sItem=p_sType;
}

void CircleObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    painter->setRenderHint(QPainter::Antialiasing,true);

    if(m_bPlayer)
    {
        int iR=_radius-4;
        //qDebug() << "P:" << MapGraphicsObject::ms_iCurRenderZoom;
        QPen pen(_fillColor);
        pen.setWidth(4);painter->setPen(pen);
        //painter->setBrush(_fillColor);
        painter->drawEllipse(QPointF(0,0),iR,iR);
        pen.setWidth(2);painter->setPen(pen);
        painter->drawLine(-iR,0,iR,0);
        painter->drawLine(0,-iR,0,iR);
        return;
    };

    if(m_bGlob)
    {
        QPen pen=painter->pen();pen.setWidth(0);painter->setPen(pen);
    };

    painter->setBrush(_fillColor);
    painter->drawEllipse(QPointF(0,0),_radius,_radius);

    if(!m_sItem.isEmpty())
    {
        static QFont xFntPrc("Tahoma",3);
        painter->setFont(xFntPrc);
        painter->drawText(-_radius,-_radius,_radius*2,_radius*2,Qt::AlignHCenter|Qt::AlignVCenter,m_sItem);
        return;
    };

    if(m_xPortal.m_eTeam!=DataCfg::T_Invalid)
    {
        float fD=S2::PosDistance(DataCfg::Get()->m_xUserCfg.m_xPos,pos());
        if(fD<=35)
        {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QColor(255,255,0));
            painter->drawEllipse(QPointF(0,0),_radius-1,_radius-1);
        };

        if(m_xPortal.m_iLevel>=0)
        {
            static QFont xFntLvl("Tahoma",5,QFont::Bold);
            painter->setFont(xFntLvl);
            painter->setPen(QColor(0,0,0));
            painter->drawText(-_radius,-_radius,_radius*2,_radius,Qt::AlignHCenter|Qt::AlignVCenter,QString::number(m_xPortal.m_iLevel));
        };
        int iCharge=m_xPortal.GetChargePrc();
        if(iCharge>=0)
        {
            static QFont xFntPrc("Tahoma",3);
            painter->setFont(xFntPrc);
            painter->drawText(-_radius,0,_radius*2,_radius,Qt::AlignHCenter|Qt::AlignBottom,QString::number(iCharge));
        };
        if(m_iKeyCnt>0)
        {
            static QColor xColK(255,255,0);
            painter->setBrush(xColK);
            QPen pen=painter->pen();pen.setWidth(0);painter->setPen(pen);
            painter->drawEllipse(QPointF(-3,0),1.5,1.5);
        }
        if(m_iResoCnt>0)
        {
            static QColor xColK(255,128,0);
            painter->setBrush(xColK);
            QPen pen=painter->pen();pen.setWidth(0);painter->setPen(pen);
            painter->drawEllipse(QPointF(3,0),1.5,1.5);
        }

        if(MapGraphicsObject::ms_iCurRenderZoom>=18)
        {
            static QColor xColW(255,255,255);
            painter->setBrush(xColW);
            QPen pen=painter->pen();pen.setWidth(0);painter->setPen(pen);
            static QFont xFntPrc("Arial",3);
            painter->setFont(xFntPrc);

            forint(i,m_xPortal.m_axResonators)
            {
                DataCfg::TPortal::TResonator& xR=m_xPortal.m_axResonators[i];
                float fA=float(xR.m_iSlot)*(PI2/8);
                float fX=cos(fA);
                float fY=-sin(fA);
                float fD=_radius+1.75;
                painter->drawEllipse(QPointF(fX*fD,fY*fD),2,2);
                if(xR.m_iSlot==4) {fY-=0.1;};
                if(xR.m_iSlot==6) {fX-=0.1;};
                painter->drawText(fX*(fD+0.3)-2,fY*(fD+0.3)-2,4,4,Qt::AlignHCenter|Qt::AlignVCenter,QString::number(xR.m_iLevel));
                //painter->fillRect(fX*(fD+0.3)-2,fY*(fD+0.3)-2,4,4,Qt::SolidPattern);
            };

            forint(i,m_xPortal.m_asMods)
            {
                float fD=_radius+1.75;
                QRectF rct;
                if(i==0) {rct.setRect(-fD,-fD-4,5,4);};
                if(i==1) {rct.setRect(+fD-5,-fD-4,5,4);};
                if(i==2) {rct.setRect(-fD,+fD,5,4);};
                if(i==3) {rct.setRect(+fD-5,+fD,5,4);};
                painter->drawRect(rct);
                painter->drawText(rct,Qt::AlignHCenter|Qt::AlignVCenter,m_xPortal.m_asMods[i].toLower());
            };
        };
        return;
    };

}

qreal CircleObject::radius() const
{
    return _radius;
}

void CircleObject::setRadius(qreal radius)
{
    _radius = radius;
    this->redrawRequested();
}

//protected
//virtual from MapGraphicsObject
void CircleObject::keyReleaseEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Delete))
    {
        this->deleteLater();
        event->accept();
    }
    else
        event->ignore();
}
