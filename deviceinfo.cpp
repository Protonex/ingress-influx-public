#include "deviceinfo.h"
#include "datacfg.h"
#include "util/scrambler.h"

DeviceInfo::DeviceInfo()
{
    board="smdk4x12";
    bootloader="I9300XXEMC2";
    brand="samsung";
    device="m0";
    deviceId=DataCfg::Get()->m_xUserCfg.GetDeviceID();
    display="JZO54K.I9300XXEMC2";
    fingerprint="samsung/m0xx/m0:4.1.2/JZO54K/I9300XXEMC2:user/release-keys";
    hardware="smdk4x12";
    manufacturer="samsung";
    model="GT-I9300";
    product="m0xx";
    rooted=false;
    tags="release-keys";
    type="user";
}

//example:
//{"board":"smdk4x12","bootloader":"I9300XXEMC2","brand":"samsung","device":"m0","deviceId":"2e568df69d9f9c24","display":"JZO54K.I9300XXEMC2","fingerprint":"samsung/m0xx/m0:4.1.2/JZO45K/I9300XXEMC2:user/release-keys","hardware":"smdk4x12","manufacturer":
//"samsung","model":"GT-I9300","product":"m0xx","rooted":false,"tags":"release-keys","type":"user"

QByteArray DeviceInfo::ToJSON()
{
    QVariantMap xM;

    QString sJ("{"
               "\"board\":\"%1\","
               "\"bootloader\":\"%2\","
               "\"brand\":\"%3\","
               "\"device\":\"%4\","
               "\"deviceId\":\"%5\","
               "\"display\":\"%6\","
               "\"fingerprint\":\"%7\","
               "\"hardware\":\"%8\","
               "\"manufacturer\":\"%9\","
               "\"model\":\"%10\","
               "\"product\":\"%11\","
               "\"rooted\":%12,"
               "\"tags\":\"%13\","
               "\"type\":\"%14\""
    "}");

    sJ=sJ
            .arg(board)
            .arg(bootloader)
            .arg(brand)
            .arg(device)
            .arg(deviceId)
            .arg(display)
            .arg(fingerprint)
            .arg(hardware)
            .arg(manufacturer)
            .arg(model)
            .arg(product)
            .arg((rooted?"true":"false"))
            .arg(tags)
            .arg(type);

    return sJ.toUtf8();
}

QString DeviceInfo::ToHandshakeEncoded()
{
    class JRand
    {
    public:
        qint64 seed;
        JRand()
        {
            seed=QDateTime::currentMSecsSinceEpoch();
            seed = ( 0x5DEECE66DL * seed + 0xBL ) & ((1L << 48) - 1);
        }
        int next(int bits)
        {
            seed = (seed * 0x5DEECE66DLL + 0xBLL) & ((1LL << 48) - 1);
            return (int) (((quint64)seed) >> (48 - bits));
        }
    };
    JRand rnd;
    //rnd.seed=1234; //TODO: delme!

    QByteArray xJ=ToJSON();
    QByteArray xE;

    //time rnd salting
    int i = xJ.size() / 252;
    int j = xJ.size() % 252;
    int k = i * 256;
    if (j > 0)
    {
        i++;
        k += j + 4;
    };
    xE.resize(k);xE.fill(0);

    char* pD=xE.data();

    int m = k;
    for (int n = 0; n < i; n++)
    {
        int iRnd=rnd.next(32);
        pD[n * 256 + 0]=(iRnd)&0xFF;
        pD[n * 256 + 1]=(iRnd>>8)&0xFF;
        pD[n * 256 + 2]=(iRnd>>16)&0xFF;
        pD[n * 256 + 3]=(iRnd>>24)&0xFF;

        int i1 = m - 4;
        int i2 = Min(i1, 252);
        memcpy(pD+4+n*256,xJ.constData()+n*252,i2);
        m = i1 - i2;
    };

    //scramble
    {
        char c[256];
        int i = 256 - (0xff & xE.size());

        int iNewSize=i+xE.size();
        QByteArray aDNew;aDNew.resize(iNewSize);
        char* abyte1 = aDNew.data();
        int j = xE.size()-256;
        int k = 0;
        while(k < xE.size())
        {
            if(k+256<xE.size())
            {
                //System.arraycopy(abyte0, k, c, 0, 256);
                memcpy(c,pD+k,256);
            }
            else
            {
                //System.arraycopy(abyte0, k, c, 0, abyte0.length - j);
                memset(c,0,256);
                memcpy(c,pD+k,j);
                //Arrays.fill(c, abyte0.length - j, 256, (byte)0);
                c[255] = (char)(i & 0xff);
            }
            //a.a(c, d);
            Nemisis_DoScramble(c);
            //System.arraycopy(d, 0, abyte1, k, 256);
            memcpy(abyte1+k,c,256);
            k += 256;
        };

        xE=aDNew;
    };

    QString sRet=xE.toBase64();
    if(sRet.endsWith('=')) {sRet=sRet.left(sRet.size()-1);};

    qDebug() << "Handshake:" << sRet;

    return sRet;
}

