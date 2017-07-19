#-------------------------------------------------
#
# Project: OGame - Plugin
#
#-------------------------------------------------

TEMPLATE         = lib
CONFIG          += plugin

include(../../global.pri)

#DEFINES         += QT_NO_DEBUG_OUTPUT

DESTDIR          = ../
TARGET           = ogame

HEADERS         += \
                ogame.h \
                account.h \
                ogamedock.h \
    fleetdialog.h

SOURCES         += \
                ogame.cpp \
                account.cpp \
                ogamedock.cpp \
    fleetdialog.cpp

FORMS           += \
                ogamedock.ui \
    fleetdialog.ui

RESOURCES       += \
                htmls/data.qrc

OTHER_FILES     += \
                htmls/constants.json \
                htmls/inject.css \
                htmls/inject.html \
                htmls/gamescript.js \
                htmls/checkscript.js

DISTFILES       += \
                htmls/constants.json
