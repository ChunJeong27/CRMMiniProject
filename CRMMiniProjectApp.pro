QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chatroomform.cpp \
    clientform.cpp \
    fileprotocol.cpp \
    logthread.cpp \
    main.cpp \
    mainwindow.cpp \
    orderform.cpp \
    productform.cpp \
    searchingdialog.cpp \
    serverform.cpp \
    uploadprotocol.cpp

HEADERS += \
    chatroomform.h \
    clientform.h \
    fileprotocol.h \
    logthread.h \
    mainwindow.h \
    orderform.h \
    productform.h \
    searchingdialog.h \
    serverform.h \
    uploadprotocol.h

FORMS += \
    chatroomform.ui \
    clientform.ui \
    mainwindow.ui \
    orderform.ui \
    productform.ui \
    searchingdialog.ui \
    serverform.ui

TRANSLATIONS += \
    CRMMiniProjectApp_ko_KR.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=
