#include "bkworkmodule.h"
#include "bkaccount.h"

#include <QNetworkReply>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

#include <QDebug>




bkWorkModule::bkWorkModule(QJsonObject& defaults, bkAccount* account, QObject *parent) :
    bkModule("Work", defaults, account, parent)
{
    addPath("/market/work");

    addStateName(waiting, tr("waiting"));
    addStateName(go_market, tr("go to market"));
    addStateName(travel, tr("reisen"));
    addStateName(go_working, tr("go working"));
    addStateName(working, tr("working"));
    addStateName(encash, tr("encash"));
}

void bkWorkModule::replyFinished(QNetworkReply* reply)
{
    QString dpath = reply->url().path();

    if(dpath == "/market/work/cancel") {
        emit jobDone();
    }
}

void bkWorkModule::parse(QWebPage* page, QVariant titleTimer, QVariant, QVariant)
{
    stopActionTimer();
    QString dpath = page->mainFrame()->url().path().toLower();

    if(state("state").toInt() == travel && !hasPath(dpath)) return;

    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement main = doc.findFirst("#mainContent");
    if(titleTimer.isValid() && !titleTimer.isNull()) {
        setState("state", waiting);
        if(main.hasClass("cooldownWork")) {
            setState("state", working);
        }
    } else {

        if(!hasPath(dpath)) {
            setState("state", go_market);
            return;
        }

        if(main.hasClass("work_singledout")) {
            QWebElement knob = doc.findFirst("#encashLink");
            if(!knob.isNull()) {
                setState("state", encash);
                return;
            }
            setState("state", go_working);

            QVariant availableSides = page->mainFrame()->evaluateJavaScript("if(typeof availableSides !== 'undefined') availableSides;");
            if(!availableSides.isValid()) return;
            QString c = "neutral";
            QVariant course = p_account->player()->value("knight_course");
            if(course.isValid()) {
                if(course.toBool()) {
                    c = "good";
                } else {
                    c = "evil";
                }
            } else {
                // get_highscore?
                // bkHighscoreModule?
            }

            /*
             * <form id="workSubmit" action="https://s12-de.battleknight.gameforge.com:443/market/work" method="POST">
             *     <input id="workFormHours" name="hours" type="hidden" value="1">
             *     <input id="workFormSide" name="side" type="hidden" value="">
             * </form>
             * side = "good", "evil", "neutral"
             */
            if(availableSides.toList().contains(c)) {
                // check paiments!
                QWebElement side = page->mainFrame()->findFirstElement("#workFormSide");
                side.setAttribute("value", c);
                //page->mainFrame()->evaluateJavaScript("document.id('workSubmit').submit();");
                qDebug() << "bkWorkModule::parse() knight_course:" << course << c;
            } else {
                // change_location?
                // bkTravelModule?
                // bkAccountModule->travelTo(BestWork)!
                setState("state", travel);
                emit hasJobFor("Account", "travel", travel_bestWork);
            }

        }
    }
}

void bkWorkModule::logic(QWebPage* page)
{
    QString dpath = page->mainFrame()->url().path().toLower();

    int strength = state("strength").toInt();
    switch(state("state").toInt()) {
        case working:
            break;
        case go_market:
            //if(dpath == "/market/work") setState("state", go_working);
            //++strength;
            break;
        case waiting:
            //setState("state", go_market);
            //strength += 2;
            break;
        case go_working:
            //strength = 48;
            break;
        case encash:
            //strength = 128;
            break;
        default:
            break;
    }
    ++strength;
    setState("strength", strength);
}

void bkWorkModule::action(QWebPage* page)
{
    QString dpath = page->mainFrame()->url().path().toLower();

    if(!state("enable").toBool()) return;
    switch(state("state").toInt()) {
        case go_market:
        case go_working:
            startActionTimer(28297); // 28sec
            break;
        case working:
            break;
        case encash:
            startActionTimer(45823); // 45sec
            break;
        default:
            break;
    }

    //qDebug() << QString("bk" + name() + "Module::action(%1, %2)").arg(stateName(state("state").toInt())).arg(state("wait").toInt()).toLocal8Bit().data() << dpath;
}

void bkWorkModule::clickMarket()
{
    QWebPage* page = p_account->currentPage();
    QString dpath = page->mainFrame()->url().path().toLower();
    if(!hasPath(dpath)) {
        QWebElement doc = page->mainFrame()->documentElement();
        QWebElement knob = doc.findFirst("#navWork");
        if(!knob.isNull()) knob.evaluateJavaScript("this.click();");
    }
}

void bkWorkModule::getCash()
{
    QWebPage* page = p_account->currentPage();
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement knob = doc.findFirst("#encashLink");
    if(!knob.isNull()) knob.evaluateJavaScript("this.click();");
}

void bkWorkModule::goWorking()
{
    QWebPage* page = p_account->currentPage();
    if(page == Q_NULLPTR) return; // tja, was tun?

    QVariant avail = page->mainFrame()->evaluateJavaScript("if(typeof availableSides !== 'undefined') availableSides;");
    if(!avail.isValid()) {
    }
    QString c = "neutral";
    QVariant course = p_account->player()->value("knight_course");
    if(course.isValid()) {
        if(course.toBool()) {
            c = "good";
        } else {
            c = "evil";
        }
    } else {
        // get_highscore?
        // bkHighscoreModule?
    }

    /*
     * <form id="workSubmit" action="https://s12-de.battleknight.gameforge.com:443/market/work" method="POST">
     *     <input id="workFormHours" name="hours" type="hidden" value="1">
     *     <input id="workFormSide" name="side" type="hidden" value="">
     * </form>
     * side = "good", "evil", "neutral"
     */
    if(avail.toList().contains(c)) {
        QWebElement side = page->mainFrame()->findFirstElement("#workFormSide");
        side.setAttribute("value", c);
        qDebug() << "bkWorkModule::goWorking() knight_course:" << course << c;
        page->mainFrame()->evaluateJavaScript("document.id('workSubmit').submit();");
    } else {
        // change_location?
        // bkTravelModule?
        // bkAccountModule->travelTo(BestWork)!
        //emit hasJobFor("Account", "travel", travel_bestWork);
    }

    //qDebug() << QString("bk" + name() + "Module::goWorking").toLocal8Bit().data();// << this;
}

void bkWorkModule::actionTimer()
{
    switch(state("state").toInt()) {
        case go_market:
            clickMarket();
            //setState("strength", 30);
            //setState("state", go_working);
            break;
        case go_working:
            goWorking();
            break;
        case encash:
            getCash();
            //setState("strength", 24);
            //setState("state", waiting);
            if(active()) emit jobDone();
            break;
        default:
            break;
    }
    qDebug() << QString("bk" + name() + "Module::actionTimer(%1)").arg(stateName(state("state").toInt())).toLocal8Bit().data();
}

void bkWorkModule::view(QWebPage* page)
{
    bkModule::view(page);
    if(!state("enable").toBool()) return;
    //QWebElement moduleElement = getModule(page->mainFrame()->documentElement());
}
