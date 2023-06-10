#ifndef TOOLS_H
#define TOOLS_H


#include <QJsonObject>
#include <QSettings>

QJsonObject convertQStringToJsonObject(QString str);
QString convertQJsonObjectToQString(QJsonObject json);
QString text_color_blue(QString str);
QString text_color_red(QString str);
QString text_color_black(QString string,QString fontstyle="font-size:20px");
QSettings *getSettings();
QStringList getIpRangeStr();
#endif // TOOLS_H
