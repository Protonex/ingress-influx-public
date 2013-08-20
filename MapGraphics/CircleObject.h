#ifndef CIRCLEOBJECT_H
#define CIRCLEOBJECT_H

#include "MapGraphics_global.h"
#include "MapGraphicsObject.h"
#include "../datacfg.h"

class MAPGRAPHICSSHARED_EXPORT CircleObject : public MapGraphicsObject
{
    Q_OBJECT
public:
    explicit CircleObject(qreal radius,bool sizeIsZoomInvariant=true, QColor fillColor = QColor(0,0,0,0),MapGraphicsObject *parent = 0);
    virtual ~CircleObject();

    //pure-virtual from MapGraphicsObject
    QRectF boundingRect() const;

    //pure-virtual from MapGraphicsObject
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    qreal radius() const;
    void setRadius(qreal radius);
    bool m_bPlayer;
    bool m_bGlob;
    QString m_sGUID;
    QString m_sItem;
    DataCfg::TPortal m_xPortal;
    int m_iKeyCnt;
    int m_iResoCnt;

    void SetPortal(QString p_sGUID, DataCfg::TPortal p_xP,int p_iKeyCnt,int p_iResoCnt);
    void SetItem(QString p_sGUID, QString p_sType);

signals:
    
public slots:

protected:
    //virtual from MapGraphicsObject
    virtual void keyReleaseEvent(QKeyEvent *event);

private:
    qreal _radius;
    QColor _fillColor;
};

#endif // CIRCLEOBJECT_H
