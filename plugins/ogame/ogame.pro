#-------------------------------------------------
#
# Project created by QtCreator 2016-05-24T03:45:25
#
#-------------------------------------------------

TEMPLATE         = lib
CONFIG          += plugin

include(../../global.pri)

#DEFINES         += QT_NO_DEBUG_OUTPUT

DESTDIR          = ../
TARGET           = ogame

SOURCES         += \
                ogame.cpp \
                account.cpp

HEADERS         += \
                ogame.h \
                account.h

OTHER_FILES     += \
                htmls/inject.css \
                htmls/inject.html \
                htmls/gamescript.js \
                htmls/checkscript.js

RESOURCES       += \
                htmls/data.qrc

DISTFILES += \
    htmls/constants.json

