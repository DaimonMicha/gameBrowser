#include "bkaccountmodule.h"
#include "bkplayer.h"

#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>



bkAccountModule::bkAccountModule(QJsonObject& defaults) :
    bkModule("Account", defaults)
{
}

void bkAccountModule::parse(QWebPage* page)
{
    checkAccount(page);
    QVariant titleTimer = page->mainFrame()->evaluateJavaScript("if(typeof l_titleTimerEndTime !== 'undefined') l_titleTimerEndTime;");
    if(titleTimer.isValid()) {
        titleTimer = titleTimer.toInt() + 1;
        setState("timerCooldown", titleTimer.toInt());
        //p_account->setStatus("timerCooldown", titleTimer);
    }
    qDebug() << "bkAccountModule::parse" << name();
}

void bkAccountModule::checkAccount(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();

    QJsonObject knight;
    QWebElement tester;

    tester = doc.findFirst("#shieldNeutral");
    if(!tester.isNull()) {
        QUrl url(tester.attribute("href"));
        knight.insert("knight_id", QJsonValue(url.path().split("/",QString::SkipEmptyParts).at(2).toInt()));
    }

    QVariant result = page->mainFrame()->evaluateJavaScript("document.id('life').retrieve('tip:title').split(', Stufe')[0];");
    if(result.isValid())
        knight.insert("knight_name", QJsonValue(result.toString()));

    tester = doc.findFirst("#userLevel");
    if(!tester.isNull())
        knight.insert("knight_level", QJsonValue(tester.toPlainText().trimmed().toInt()));

    tester = doc.findFirst("#lifeCount");
    if(!tester.isNull())
        knight.insert("live_count", QJsonValue(tester.toPlainText().trimmed().toInt()));

    result = page->mainFrame()->evaluateJavaScript("g_maxHealth;");
    if(result.isValid())
        knight.insert("max_live", QJsonValue(result.toInt()));

    tester = doc.findFirst("#levelCount");
    if(!tester.isNull())
        knight.insert("experience", QJsonValue(tester.toPlainText().trimmed().toInt()));

    tester = doc.findFirst("#silverCount");
    if(!tester.isNull())
        knight.insert("silver", QJsonValue(tester.toPlainText().trimmed().toInt()));

    result = page->mainFrame()->evaluateJavaScript("document.id('silver').retrieve('tip:text').split(': ')[1];");
    if(result.isValid())
        knight.insert("treasury", QJsonValue(result.toInt()));

    tester = doc.findFirst("#rubyCount");
    if(!tester.isNull())
        knight.insert("rubies", QJsonValue(tester.toPlainText().trimmed().toInt()));

    tester = doc.findFirst("body");
    if(!tester.isNull()) {
        if(tester.classes().contains("evil")) knight.insert("knight_course",QJsonValue(false));
        else knight.insert("knight_course",QJsonValue(true));
    }

    //bkPlayer* p = p_account->world()->player(knight.value("knight_id").toInt());
    //p->setData(knight);

    tester = doc.findFirst("#contentTitle");
    if(tester.isNull()) return;
    qDebug() << "\tbkParser::checkAccount(#contentTitle)" << tester.toPlainText().trimmed();
}

void bkAccountModule::logic(QWebPage*)
{
    qDebug() << "bkAccountModule::logic" << name();
}

void bkAccountModule::view(QWebPage*)
{
}
