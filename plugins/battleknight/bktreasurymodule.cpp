#include "bktreasurymodule.h"

#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>




bkTreasuryModule::bkTreasuryModule(QJsonObject& defaults, bkAccount* account, QObject *parent) :
    bkModule("Treasury", defaults, account, parent)
{
    addPath("/treasury");
    addPath("/treasury/index");
}

void bkTreasuryModule::parse(QWebPage* page, QVariant, QVariant progressbarEndTime, QVariant)
{
    QString dpath = page->mainFrame()->url().path().toLower();
    if(!hasPath(dpath)) return;

    if(progressbarEndTime.isValid() && progressbarEndTime.toInt() > 0) {
        setState("timerProgress", progressbarEndTime.toInt()+3);
    } else {
        setState("timerProgress", 0);
    }

    if(active()) emit jobDone();
}

void bkTreasuryModule::logic(QWebPage* page)
{
    //qDebug() << "bkTreasuryModule::logic(timerProgress)" << state("timerProgress").toInt();
    QString dpath = page->mainFrame()->url().path().toLower();
}

void bkTreasuryModule::action(QWebPage*)
{
    // #btnTreasuryPayin
}

void bkTreasuryModule::view(QWebPage* page)
{
    bkModule::view(page);
    if(!enabled()) return;
    QWebElement moduleElement = getModule(page->mainFrame()->documentElement());

    QVariant result = state("timerProgress").toVariant();
    if(result.isValid() && result.toInt() > 0) addProgressBar(moduleElement, "Progress");

    //qDebug() << QString("bk"+name()+"Module::view").toLocal8Bit().data() << this;
}
