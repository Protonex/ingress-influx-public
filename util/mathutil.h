#ifndef MATHUTIL_H
#define MATHUTIL_H

static const double PI2=6.28318530718;
static const float PI2F=6.28318530718f;
static const double PI=3.1415926535897932384626433832795;
static const float PIF=3.1415926535897932384626433832795f;
static const double PIHALF=1.5707963268;
static const float PIHALFF=1.5707963268f;
static const double PIQUATER=0.7853981633975;
static const float PIQUATERF=0.7853981633975f;
static const double PI3QUATER=2.356194490192;
static const float PI3QUATERF=2.356194490192f;
static const double PI15=4.712388980385;
static const float PI15F=4.712388980385f;

static const double SQRT2=1.41421356237;
static const float SQRT2F=1.41421356237f;
static const double SQRT05=0.707106781187;
static const float SQRT05F=0.707106781187f;

static const double LOG2=0.69314718055994530941723212145818;
static const float LOG2F=0.69314718055994530941723212145818f;
static const double LOG10_2=0.30102999566398119521373889472449;
static const float LOG10_2F=0.30102999566398119521373889472449f;

static const float FLTMAX=3.402823466e+38F;
static const float FLTMAXT=3.402823466e+37F;
static const float FLTMIN=1.175494351e-38F;
static const float FLTEPSILON=1.192092896e-07F;

static const double DBLMAX=1.7976931348623158e+308;
static const double DBLMIN=2.2250738585072014e-308;

static const int INT32MIN=(-2147483647-1);
static const int INT32MAX=2147483647;
static const qint64 INT64MAX=9223372036854775807ll;
static const qint64 INT64MIN=(-9223372036854775807ll - 1);

inline static float Deg2Rad(float fV) {return (fV*PIF)/180.0f;}
inline static double Deg2Rad(double fV) {return (fV*PI)/180.0;}

inline static float Rad2Deg(float fV) {return (fV/PIF)*180.0f;}
inline static double Rad2Deg(double fV) {return (fV/PI)*180.0;}

//static float drem(float a,float b) {return (a - int(a/b)* b);}
//static double drem(double a,double b) {return (a - int(a/b)* b);}

template<class c>
const c& Max(const c& a,const c& b) {return a>b?a:b;}
template<class c>
const c& Min(const c& a,const c& b) {return a>b?b:a;}

#define forint(variable,container) for(int variable=0;variable<container.size();variable++)

//slit string but ignore escaped separator (fake SplitCStyle)
QStringList SplitString(const QString& s,const QString &sep, QString::SplitBehavior behavior=QString::KeepEmptyParts, Qt::CaseSensitivity cs=Qt::CaseSensitive);
void StringListSortCaseInsensitive(QStringList& p_asVals);
QByteArray ReadFile(QString p_sFileName);

//see: http://lists.qt.nokia.com/pipermail/qt-interest/2009-December/016226.html
template <typename T>
class DelayedDelete : public QObject
{
public:
	explicit DelayedDelete(T *&item) : m_item(item)
	{
		item = 0;
		deleteLater();
	}
	virtual ~DelayedDelete()
	{
		delete m_item;
	}
private:
	T *m_item;
};

class AutoBoolRelease
{
	bool& m_b;
public:
	AutoBoolRelease(bool& b) : m_b(b) {}
	~AutoBoolRelease() {m_b=false;}
};

class AutoIntIncDec
{
	int& m_i;
public:
	AutoIntIncDec(int& i) : m_i(i) {m_i++;}
	~AutoIntIncDec() {m_i--;}
};

///// src\gui\painting\qdrawhelper_p.h

/*#define INV_PREMUL(p)                                   \
	(qAlpha(p) == 0 ? 0 :                               \
	((qAlpha(p) << 24)                                  \
	 | (((255*qRed(p))/ qAlpha(p)) << 16)               \
	 | (((255*qGreen(p)) / qAlpha(p))  << 8)            \
	 | ((255*qBlue(p)) / qAlpha(p))))*/

static inline uint INV_PREMUL(uint x) {
	uint a = x >> 24;
	if(a==0) {return 0;};
	return (a<<24)
			|(((255*qRed(x))/a) << 16)
			|(((255*qGreen(x))/a)  << 8)
			| ((255*qBlue(x))/a);
}

#if QT_POINTER_SIZE == 8 // 64-bit versions
static inline uint PREMUL(uint x) {
	uint a = x >> 24;
	quint64 t = (((quint64(x)) | ((quint64(x)) << 24)) & 0x00ff00ff00ff00ff) * a;
	t = (t + ((t >> 8) & 0xff00ff00ff00ff) + 0x80008000800080) >> 8;
	t &= 0x000000ff00ff00ff;
	return (uint(t)) | (uint(t >> 24)) | (a << 24);
}
#else // 32-bit versions
static inline uint PREMUL(uint x) {
	uint a = x >> 24;
	uint t = (x & 0xff00ff) * a;
	t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
	t &= 0xff00ff;

	x = ((x >> 8) & 0xff) * a;
	x = (x + ((x >> 8) & 0xff) + 0x80);
	x &= 0xff00;
	x |= t | (a << 24);
	return x;
}
#endif


#endif // MATHUTIL_H
