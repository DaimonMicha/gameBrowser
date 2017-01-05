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
TARGET           = plugin

SOURCES         += \
                plugin.cpp \
                account.cpp

HEADERS         += \
                plugin.h \
                account.h

OTHER_FILES     += \
                htmls/inject.css \
                htmls/inject.html \
                htmls/gamescript.js \
                htmls/checkscript.js

RESOURCES       += \
                htmls/data.qrc

