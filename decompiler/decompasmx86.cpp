#include "decompasmx86.h"
#include "../pch.h"

DecompAsmX86::DecompAsmX86()
{
}

static QString ParseArgAddr(QString p_sArg)
{
    if(p_sArg.startsWith("dword ptr "))
    {
        p_sArg=p_sArg.mid(10);
    };
    if(!p_sArg.startsWith("[")) {return p_sArg;};

    int iA=p_sArg.indexOf("arg_");
    if(iA!=-1)
    {
        QString sA=p_sArg.mid(iA,p_sArg.length()-iA-1);
        return "(int)"+sA;
    };
    int iV=p_sArg.indexOf("var_");
    if(iV!=-1)
    {
        QString sV=p_sArg.mid(iV,p_sArg.length()-iV-1).toLower();
        int iP=sV.indexOf("+");
        if(iP==-1) {return sV;};

        QString sO=sV.mid(iP+1);
        if(sO.endsWith("h")) {sO=sO.left(sO.length()-1);};
        int iOfs=sO.toInt(0,16);
        sV=sV.left(iP);

        iOfs=sV.mid(4).toInt(0,16)-iOfs;
        sV="var_"+QString::number(iOfs,16);
        return sV;
    };

    p_sArg=p_sArg.mid(1,p_sArg.length()-2);
    int iP=p_sArg.indexOf("+");
    int iOfs=0;
    if(iP!=-1)
    {
        QString sO=p_sArg.mid(iP+1);
        if(sO.endsWith("h")) {sO=sO.left(sO.length()-1);};
        iOfs=sO.toInt(0,16);
        p_sArg=p_sArg.left(iP);
    };

    if(iOfs==0)
    {
        return "*((int*)"+p_sArg+")";
    }
    else
    {
        return "*((int*)("+p_sArg+"+0x"+QString::number(iOfs,16)+"))";
    };

    return p_sArg;
}

void DecompAsmX86::Do()
{
    QFile xFileI("../influx/decompiler/libnemisis_x68.asm");
    if(!xFileI.open(QIODevice::ReadOnly))
    {
        qError()<<"mööp!";
        return;
    };

    QString sF=QString::fromLatin1(xFileI.readAll());
    QStringList asF=sF.split("\n");

    QString sOut;
    QString sCurSub;
    bool bIsDataSub=false;
    QList<int> aiParams;
    QList<int> aiVars;
    QStringList asDecls;

    forint(i,asF)
    {
        QString sL=asF[i];
        int iS=sL.indexOf(" ");if(iS==-1) {continue;};
        sL=sL.mid(iS+1);

        if(sL.startsWith("sub_"))
        {
            if(sL.contains("endp"))
            {
                sCurSub.clear();
                aiParams.clear();
                aiVars.clear();
                if(bIsDataSub)
                {
                    sOut+="}\n\n";
                };
                bIsDataSub=false;
                continue;
            };

            sCurSub=sL.split(" ").first();
            bIsDataSub=sL.contains("DATA XREF: .data");
            if(bIsDataSub)
            {
                qDebug() << "Sub found: " << sCurSub;
            };
        };
        if(sCurSub.isEmpty()||!bIsDataSub) {continue;};

        if(sL.startsWith("var_"))
        {
            QString sH=sL.split(" ").first().mid(4);
            aiVars.append(sH.toInt(0,16));
            continue;
        };
        if(sL.startsWith("arg_"))
        {
            QString sH=sL.split(" ").first().mid(4);
            aiParams.append(sH.toInt(0,16));
            continue;
        };
        sL=sL.trimmed();
        if(sL.startsWith("sub     esp,"))
        {
            QString sDecl;

            sDecl+="static void "+sCurSub+"(";
            forint(j,aiParams)
            {
                sDecl+="char* arg_"+QString::number(aiParams[j],16);
                if(j<aiParams.size()-1) {sDecl+=",";};
            };
            sDecl+=")";

            sOut+=sDecl;
            sOut+="\n{\n";
            sOut+="\tint eax,ebc,ecx,edx,edi,esi,ebp;\n\t";
            forint(j,aiVars)
            {
                sOut+="int var_"+QString::number(aiVars[j],16)+";";
            };
            sOut+="\n";

            asDecls.append(sDecl);

            continue;
        };
        if(sL.startsWith("mov"))
        {
            sL=sL.mid(8);
            int iC=sL.indexOf(",");if(iC==-1) {continue;};
            QString sDst=sL.left(iC);
            QString sSrc=sL.mid(iC+1).trimmed();

            sOut+="\t"+ParseArgAddr(sDst)+"="+ParseArgAddr(sSrc)+";\n";
        };
        if(sL.startsWith("xor"))
        {
            sL=sL.mid(8);
            int iC=sL.indexOf(",");if(iC==-1) {continue;};
            QString sDst=sL.left(iC);
            QString sSrc=sL.mid(iC+1).trimmed();

            sOut+="\t"+ParseArgAddr(sDst)+"^="+ParseArgAddr(sSrc)+";\n";
        };
        if(sL.startsWith("and"))
        {
            sL=sL.mid(8);
            int iC=sL.indexOf(",");if(iC==-1) {continue;};
            QString sDst=sL.left(iC);
            QString sSrc=sL.mid(iC+1).trimmed();

            sOut+="\t"+ParseArgAddr(sDst)+"&="+ParseArgAddr(sSrc)+";\n";
        };
        if(sL.startsWith("or"))
        {
            sL=sL.mid(8);
            int iC=sL.indexOf(",");if(iC==-1) {continue;};
            QString sDst=sL.left(iC);
            QString sSrc=sL.mid(iC+1).trimmed();

            sOut+="\t"+ParseArgAddr(sDst)+"|="+ParseArgAddr(sSrc)+";\n";
        };
        if(sL.startsWith("not"))
        {
            sL=sL.mid(8);
            QString sDst=sL;

            sOut+="\t"+ParseArgAddr(sDst)+"=~"+ParseArgAddr(sDst)+";\n";
        };

        //qDebug() << sL;
    };

    //qDebug() << sF;
    QString sHead;
    forint(i,asDecls)
    {
        sHead+=asDecls[i]+";\n";
    };
    sOut=sHead+"\n\n"+sOut;

    QFile xFileO("../influx/decompiler/libnemisis_x68.c");
    if(!xFileO.open(QIODevice::WriteOnly))
    {
        qError()<<"plopp!";
        return;
    };
    xFileO.write(sOut.toLatin1());
}
