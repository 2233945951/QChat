#include "tools.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>
QJsonObject convertQStringToJsonObject(QString str)
{
    QJsonDocument jsonDocument;
#if defined (Q_OS_WIN)
    jsonDocument=QJsonDocument::fromJson(str.toUtf8());
#else
    jsonDocument=QJsonDocument::fromJson(str.toLocal8Bit());
#endif

    if(!jsonDocument.isNull()){
        return jsonDocument.object();
    }
    return *new QJsonObject;
}
QString convertQJsonObjectToQString(QJsonObject json)
{
    return QString(QJsonDocument(json).toJson());
}
QString text_color_blue(QString str)
{
    return "<font size=5 color=blue>"+str+"</font> <font color=black> </font>";
}

QString text_color_red(QString str)
{
    return "<font size=5 color=red>"+str+"</font> <font color=black> </font>";
}

QString text_color_black(QString string,QString fontstyle)
{
    QString str="<font color=black style=\"%1\">%2</font> <font color=black> </font>";
    return str.arg(fontstyle,string);
}


QSettings *getSettings()
{
    QString dir=QDir::currentPath();
    QSettings *settings=new QSettings(QString("%1/config.conf").arg(dir),QSettings::IniFormat);
    return settings;
}

QStringList getIpRangeStr()
{
    QList<QNetworkAddressEntry> entry;
    QStringList ipList;

    for(const auto &i:QNetworkInterface::allInterfaces()){

        //如果是Windows系统，有线网卡一般以ethernet开头，无线网卡以wireless开头
#if defined (Q_OS_WIN)
        if(i.name().indexOf("ethernet")>-1 || i.name().indexOf("wireless")>-1)
            entry.append(i.addressEntries());

#else
        if(i.name().indexOf("en")>-1 || i.name().indexOf("wl")>-1)
            entry.append(i.addressEntries());
#endif
    }
    for (int i(0);i<entry.count() ;i++ ) {
        if(entry.at(i).ip().protocol()==QAbstractSocket::IPv4Protocol){
            ipList.append(entry.at(i).ip().toString());
        }
    }
    return ipList;
}
