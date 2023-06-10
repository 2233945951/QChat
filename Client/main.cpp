#include "clientmsg.h"
#include "login.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "clientmsg.h"
int main(int argc, char *argv[])
{
    QApplication *a=new QApplication(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "QtNT_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a->installTranslator(&translator);
            break;
        }
    }

      ClientMsg msg;
      msg.show();

//    PrivateChat chat;
//    chat.show();


    return a->exec();
}
