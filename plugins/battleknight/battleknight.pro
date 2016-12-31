#-------------------------------------------------
#
# Project created by QtCreator 2016-05-24T03:45:25
#
#-------------------------------------------------

TEMPLATE         = lib
CONFIG          += plugin

include(../../global.pri)

#DEFINES         += QT_NO_DEBUG_OUTPUT

QT              += widgets network webkit webkitwidgets

DESTDIR          = ../
TARGET           = battleknight

SOURCES         += \
                battleknight.cpp \
                account.cpp \
                itemmanager.cpp \
                playermanager.cpp \
    reportmanager.cpp

HEADERS         += \
                battleknight.h \
                account.h \
                itemmanager.h \
                playermanager.h \
    reportmanager.h

OTHER_FILES     += \
                htmls/inject.css \
                htmls/inject.html \
                htmls/gamescript.js \
                htmls/checkscript.js \
                htmls/locations.json

RESOURCES       += \
                htmls/data.qrc

