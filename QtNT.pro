QT       += core gui sql network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    client_thread.cpp \
    customqtable.cpp \
    forgot_password.cpp \
    login.cpp \
    main.cpp \
    clientmsg.cpp \
    mulitclient.cpp \
    privatechat.cpp \
    register.cpp \
    resetpassword.cpp \
    tools.cpp

HEADERS += \
    client_thread.h \
    clientmsg.h \
    customqtable.h \
    forgot_password.h \
    login.h \
    mulitclient.h \
    privatechat.h \
    register.h \
    resetpassword.h \
    tools.h

FORMS += \
    clientmsg.ui \
    forgot_password.ui \
    login.ui \
    privatechat.ui \
    register.ui \
    resetpassword.ui

CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \
    sendAuthCode.py
