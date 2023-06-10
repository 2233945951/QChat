QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH+=/usr/include/python3.8/


DESTDIR_TARGET+=../

SOURCES += \
    customdelegate.cpp \
    main.cpp \
    mulitimain.cpp \
    server.cpp \
    server_thread.cpp \
    tools.cpp \
    usermanager.cpp \
    welcome.cpp

HEADERS += \
    customdelegate.h \
    headers.h \
    mulitimain.h \
    server.h \
    server_thread.h \
    tools.h \
    usermanager.h \
    welcome.h

FORMS += \
    mulitimain.ui \
    server.ui \
    usermanager.ui \
    welcome.ui

TRANSLATIONS += \
    QtNT_Server_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

LIBS+= -L /usr/lib/python3.8/config-3.8-x86_64-linux-gnu/ -lpython3.8
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
