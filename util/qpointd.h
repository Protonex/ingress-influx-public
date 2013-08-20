#ifndef QPOINTD_H
#define QPOINTD_H

#include <QtCore/qnamespace.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Core)


class Q_CORE_EXPORT QPointD
{
public:
    QPointD();
    QPointD(const QPoint &p);
    QPointD(double xpos, double ypos);

    double manhattanLength() const;

    bool isNull() const;

    double x() const;
    double y() const;
    void setX(double x);
    void setY(double y);

    double &rx();
    double &ry();

    QPointD &operator+=(const QPointD &p);
    QPointD &operator-=(const QPointD &p);
    QPointD &operator*=(double c);
    QPointD &operator/=(double c);

    friend inline bool operator==(const QPointD &, const QPointD &);
    friend inline bool operator!=(const QPointD &, const QPointD &);
    friend inline const QPointD operator+(const QPointD &, const QPointD &);
    friend inline const QPointD operator-(const QPointD &, const QPointD &);
    friend inline const QPointD operator*(double, const QPointD &);
    friend inline const QPointD operator*(const QPointD &, double);
    friend inline const QPointD operator-(const QPointD &);
    friend inline const QPointD operator/(const QPointD &, double);

    QPoint toPoint() const;

private:
    friend class QMatrix;
    friend class QTransform;

    double xp;
    double yp;
};

Q_DECLARE_TYPEINFO(QPointD, Q_MOVABLE_TYPE);

/*****************************************************************************
  QPointD stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QPointD &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QPointD &);
#endif

/*****************************************************************************
  QPointD inline functions
 *****************************************************************************/

inline QPointD::QPointD() : xp(0), yp(0) { }

inline QPointD::QPointD(double xpos, double ypos) : xp(xpos), yp(ypos) { }

inline QPointD::QPointD(const QPoint &p) : xp(p.x()), yp(p.y()) { }

inline bool QPointD::isNull() const
{
    return qIsNull(xp) && qIsNull(yp);
}

inline double QPointD::x() const
{
    return xp;
}

inline double QPointD::y() const
{
    return yp;
}

inline void QPointD::setX(double xpos)
{
    xp = xpos;
}

inline void QPointD::setY(double ypos)
{
    yp = ypos;
}

inline double &QPointD::rx()
{
    return xp;
}

inline double &QPointD::ry()
{
    return yp;
}

inline QPointD &QPointD::operator+=(const QPointD &p)
{
    xp+=p.xp;
    yp+=p.yp;
    return *this;
}

inline QPointD &QPointD::operator-=(const QPointD &p)
{
    xp-=p.xp; yp-=p.yp; return *this;
}

inline QPointD &QPointD::operator*=(double c)
{
    xp*=c; yp*=c; return *this;
}

inline bool operator==(const QPointD &p1, const QPointD &p2)
{
    return qFuzzyIsNull(p1.xp - p2.xp) && qFuzzyIsNull(p1.yp - p2.yp);
}

inline bool operator!=(const QPointD &p1, const QPointD &p2)
{
    return !qFuzzyIsNull(p1.xp - p2.xp) || !qFuzzyIsNull(p1.yp - p2.yp);
}

inline const QPointD operator+(const QPointD &p1, const QPointD &p2)
{
    return QPointD(p1.xp+p2.xp, p1.yp+p2.yp);
}

inline const QPointD operator-(const QPointD &p1, const QPointD &p2)
{
    return QPointD(p1.xp-p2.xp, p1.yp-p2.yp);
}

inline const QPointD operator*(const QPointD &p, double c)
{
    return QPointD(p.xp*c, p.yp*c);
}

inline const QPointD operator*(double c, const QPointD &p)
{
    return QPointD(p.xp*c, p.yp*c);
}

inline const QPointD operator-(const QPointD &p)
{
    return QPointD(-p.xp, -p.yp);
}

inline QPointD &QPointD::operator/=(double c)
{
    xp/=c;
    yp/=c;
    return *this;
}

inline const QPointD operator/(const QPointD &p, double c)
{
    return QPointD(p.xp/c, p.yp/c);
}

inline QPoint QPointD::toPoint() const
{
    return QPoint(qRound(xp), qRound(yp));
}

#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug d, const QPointD &p);
#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif // QPOINTD_H
