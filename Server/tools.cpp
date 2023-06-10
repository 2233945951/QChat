#include "tools.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
QJsonObject convertQStringToJsonObject(QString str)
{
    QJsonDocument jsonDocument=QJsonDocument::fromJson(str.toLocal8Bit());
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


QJsonObject toSendCmd()
{
    QJsonObject *json=new QJsonObject;
    json->insert("type","cmd");
    json->insert("data","transfer");
    return *json;
}


QSettings *getSetting()
{
    QString dir=QDir::currentPath();
    QSettings *settings=new QSettings(QString("%1/config.conf").arg(dir),QSettings::IniFormat);
    return settings;
}

QByteArray toSendAuthCodeReply(bool flag)
{
    QJsonObject *json=new QJsonObject;
    json->insert("type","cmd");
    json->insert("cmd_type","forgot_password");
    if(flag)
        json->insert("data","CODE_ACK_SUCC");
    else
        json->insert("data","CODE_ACK_Err");
    return convertQJsonObjectToQString(*json).toLocal8Bit();
}

QByteArray toSendAuthResetReply(bool flag)
{
    QJsonObject *json=new QJsonObject;
    json->insert("type","cmd");
    json->insert("cmd_type","forgot_password");
    if(flag)
        json->insert("data","AUTH_ACK_SUCC");
    else
        json->insert("data","AUTH_ACK_Err");
    return convertQJsonObjectToQString(*json).toLocal8Bit();
}

QByteArray toSendHeartbeatData()
{
    QJsonObject *json=new QJsonObject;
    json->insert("type","cmd");
    json->insert("cmd_type","heartbeat");
    json->insert("data","SYN");
    return convertQJsonObjectToQString(*json).toLocal8Bit();
}
