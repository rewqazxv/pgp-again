QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
PRECOMPILED_HEADER = src/pch.hpp

SOURCES += \
    src/encrypt/aes.c \
    src/encrypt/rsa.cpp \
    src/main.cpp \
    src/ui/mainwindow.cpp \
    src/tools.cpp

INCLUDEPATH += \
    src/

HEADERS += \
    src/encrypt/aes.h \
    src/encrypt/rsa.hpp \
    src/ui/mainwindow.h \
    src/tools.h

FORMS += \
    src/ui/mainwindow.ui

TRANSLATIONS += \
    src/translations/zh_CN.ts

RESOURCES += \
    src/res.qrc

LIBS += \
    -lgmpxx \
    -lgmp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
