#-------------------------------------------------
#
# Project created by QtCreator 2016-05-24T03:45:25
#
#-------------------------------------------------

include(../../global.pri)

DESTDIR          = ../
TARGET           = battleknight
TEMPLATE         = lib
CONFIG          += plugin

#DEFINES         += QT_NO_DEBUG_OUTPUT

SOURCES         += \
                battleknight.cpp \
                #accountui.cpp \
                account.cpp \
                itemmanager.cpp \
                playermanager.cpp \
                reportmanager.cpp \
                battleknightdock.cpp \
                unserializephp.cpp \
                bkplayer.cpp \
    bkworld.cpp \
    bkaccount.cpp \
    bkparser.cpp \
    bkplayermanager.cpp \
    bkitem.cpp \
    bkitemmanager.cpp \
    bkclan.cpp \
    bkclanmanager.cpp \
    bkreport.cpp \
    bkreportmanager.cpp \
    bklogic.cpp \
    bkmodulemanager.cpp \
    bkmodule.cpp \
    bkaccountmodule.cpp \
    bkduelsmodule.cpp \
    bkgmmodule.cpp \
    bkmissionsmodule.cpp \
    bktreasurymodule.cpp \
    bkosmodule.cpp \
    bktournamentmodule.cpp \
    bkworkmodule.cpp \
    bkreportsmodule.cpp \
    bkhighscoremodule.cpp \
    bktravelmodule.cpp

HEADERS         += \
                battleknight.h \
                #accountui.h \
                account.h \
                itemmanager.h \
                playermanager.h \
                reportmanager.h \
                battleknightdock.h \
                unserializephp.h \
                bkplayer.h \
    bkworld.h \
    bkaccount.h \
    bkparser.h \
    bkplayermanager.h \
    bkitem.h \
    bkitemmanager.h \
    bkclan.h \
    bkclanmanager.h \
    bkreport.h \
    bkreportmanager.h \
    bklogic.h \
    bkmodulemanager.h \
    bkmodule.h \
    bkaccountmodule.h \
    bkduelsmodule.h \
    bkgmmodule.h \
    bkmissionsmodule.h \
    bktreasurymodule.h \
    bkosmodule.h \
    bktournamentmodule.h \
    bkworkmodule.h \
    bkreportsmodule.h \
    bkhighscoremodule.h \
    bktravelmodule.h

RESOURCES       += \
                htmls/data.qrc

FORMS           += \
                battleknightdock.ui

OTHER_FILES     += \
                battleknight.json \
                htmls/inject.css \
                htmls/inject.html \
                htmls/gamescript.js \
                htmls/checkscript.js \
                htmls/locations.json

DISTFILES += \
    htmls/karma.json \
    htmls/modules.json
