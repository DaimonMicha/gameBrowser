#include "bkosmodule.h"

#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>




bkOSModule::bkOSModule(QJsonObject& defaults, bkAccount* account, QObject *parent) :
    bkModule("OS", defaults, account, parent)
{
}

void bkOSModule::parse(QWebPage* page, QVariant, QVariant progressbarEndTime, QVariant progressbarDuration)
{
    QStringList paths = page->mainFrame()->url().path().split("/", QString::SkipEmptyParts);
    QString first = paths.takeFirst();

    if(first == "tavern") {
        if(!paths.isEmpty()) {
            QString second = paths.takeFirst();
            if(second == "prepare") {// Kriegsvorbereitung
                tavernPrepareHtml(page, progressbarEndTime, progressbarDuration);
            } else if(second == "battle") {// Krieg
                tavernBattleHtml(page, progressbarEndTime, progressbarDuration);
            } else if(second == "fight") {// Kämpfe
            }
        }
    } else if(first == "clanwar") {
        if(!paths.isEmpty()) {
            QString second = paths.takeFirst();
            if(second == "prepare") {// Kriegsvorbereitung
                tavernPrepareHtml(page, progressbarEndTime, progressbarDuration);
            } else if(second == "battle") {// Krieg
                tavernBattleHtml(page, progressbarEndTime, progressbarDuration);
            } else if(second == "fight") {// Kämpfe
            }
        }
    }
}

void bkOSModule::tavernBattleHtml(QWebPage* page, QVariant progressbarEndTime, QVariant progressbarDuration)
{
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement main = doc.findFirst("#mainContent");
    if(main.isNull()) return;
    if(!main.classes().contains("battle")) return;
    setState("timerProgress", progressbarEndTime.toInt());
    setState("timerProgressDuration", progressbarDuration.toInt());
    QWebElement battlerounds = main.findFirst("div.battlerounds");
    if(battlerounds.isNull()) return;
    QString round = battlerounds.toPlainText().trimmed().split(" ", QString::SkipEmptyParts).at(1);
    setState("state", round.toInt() + 2);
    qDebug() << "bkOSModule::tavernBattleHtml" << progressbarEndTime.toInt() << progressbarDuration.toInt() << round.toInt();
}

void bkOSModule::tavernPrepareHtml(QWebPage* page, QVariant progressbarEndTime, QVariant progressbarDuration)
{
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement main = doc.findFirst("#mainContent");
    if(main.isNull()) return;
    if(!main.classes().contains("prepare")) return;
    setState("timerProgress", progressbarEndTime.toInt());
    setState("timerProgressDuration", progressbarDuration.toInt());
    setState("state", 2);// eingezogen
    qDebug() << "bkOSModule::tavernPrepareHtml" << progressbarEndTime.toInt() << progressbarDuration.toInt();
}

void bkOSModule::logic(QWebPage*)
{
}

void bkOSModule::action(QWebPage*)
{
}

void bkOSModule::view(QWebPage* page)
{
    bkModule::view(page);
    if(!state("enable").toBool()) return;
    QWebElement moduleElement = getModule(page->mainFrame()->documentElement());

    QVariant result = state("timerProgress").toVariant();
    if(result.isValid() && result.toInt() > 0) {
        QString text = QString("%1. Runde").arg(state("state").toInt()-2);
        addProgressBar(moduleElement, "Progress", text);
    }

    //qDebug() << QString("bk"+name()+"Module::view").toLocal8Bit().data() << this;
}
