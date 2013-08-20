#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include "../pch.h"

class DeviceInfo
{
public:
    DeviceInfo();

    QString board;
    QString bootloader;
    QString brand;
    QString device;
    QString deviceId;
    QString display;
    QString fingerprint;
    QString hardware;
    QString manufacturer;
    QString model;
    QString product;
    bool rooted;
    QString tags;
    QString type;

    QByteArray ToJSON();
    QString ToHandshakeEncoded();
};

#endif // DEVICEINFO_H
