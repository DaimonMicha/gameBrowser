#include "bkmissionsmodule.h"
#include "bkaccount.h"

#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrlQuery>

#include <QDebug>




bkMissionsModule::bkMissionsModule(QJsonObject& defaults, bkAccount* account, QObject *parent) :
    bkModule("Missions", defaults, account, parent)
{
    addPath("/world");
    addPath("/world/location");

    addStateName(no_points, tr("no points"));
    addStateName(get_reward, tr("get reward"));

    connect(&mTimer, SIGNAL(timeout()), this, SLOT(adjustPoints()));
    QTimer::singleShot(100, this, SLOT(adjustTimer()));
}

void bkMissionsModule::restoreState(QJsonObject& state)
{
    bkModule::restoreState(state);
    if(state.contains("location")) {
        QVariant location = state.value("location").toString();
        p_account->setStatus("location", location);
    }
}

void bkMissionsModule::adjustTimer()
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    setState("timerProgress", (int)now.addSecs(3600).toTime_t());
    mTimer.start(3600 * 1000);// 1h
    qDebug() << QString("bk" + name() + "Module::adjustTimer(%1)").arg(state("timerProgress").toInt()).toLocal8Bit().data();
}

void bkMissionsModule::adjustPoints()
{
    int ist = state("points").toInt();
    int maxPoints = 120;
    int count = 5;
    QVariant prem = p_account->player()->value("manor_royalty");

    if(prem.isValid() && prem.toBool()) {
        maxPoints = 240;
        count = 10;
    }
    if(ist < maxPoints) {
        ist += count;
        setState("points", ist);
        // MPUpdate aufrufen
        QWebPage* page = p_account->currentPage();
        page->mainFrame()->evaluateJavaScript("if(typeof MPUpdate === 'function') MPUpdate();");
        qDebug() << QString("bk" + name() + "Module::adjustPoints(%1)").arg(ist).toLocal8Bit().data();
    }
}

void bkMissionsModule::replyFinished(QNetworkReply* reply)
{
    QStringList paths = reply->url().path().split("/", QString::SkipEmptyParts);
    if(!paths.count()) return; // nothing to do
    QString first = paths.takeFirst();

    if(first == "world") {
        if(!paths.isEmpty()) {
            QString second = paths.takeFirst();
            if(second == "location") { // /world/location
                QUrlQuery query(reply->property("postData").toString());
                if(!query.hasQueryItem("missionArt")) return;
                if(query.queryItemValue("buyRubies").toInt() > 0) return;
                int p = state("points").toInt();
                QString art = query.queryItemValue("missionArt");
                if(art == "small") {
                    p -= 20;
                } else if(art == "medium") {
                    p -= 40;
                } else if(art == "large") {
                    p -= 60;
                }
                if(p < 0) return;
                setState("points", p);
            }
        }
    }
    //QByteArray data = reply->property("getData").toByteArray();
}

void bkMissionsModule::parse(QWebPage* page, QVariant, QVariant, QVariant)
{
    QString dpath = page->mainFrame()->url().path().toLower();
    if(!hasPath(dpath)) return;

    QWebElement document = page->mainFrame()->documentElement();
    QWebElement main = document.findFirst("#mainContent");
    if(main.isNull()) return;

    // Location
    if(main.classes().contains("location")) {

        QWebElement news = document.findFirst("span#devCountNewMails");
        if(!news.isNull()) {
            //qDebug() << QString("bkMissionsModule::parse(has some world-news): %1").arg(news.toPlainText().trimmed().toInt()).toLocal8Bit().data();
        }

        QVariant location = main.classes().last();
        setState("location", location.toString());
        p_account->setStatus("location", location);
        // Missions
        //
        QWebElementCollection links = main.findAll("a");
        foreach(QWebElement link, links) {
            if(link.hasClass("mission")) {
                // Dragon-Event
                // neue strategie: jage den großen Drachen!
                if(link.attribute("id") == "DragonEventGreyDragon") {
                } else if(link.attribute("id") == "DragonEventGreatDragon") {
                }
                qDebug() << "Mission:" << link.attribute("id") << link.classes();
            }
        }

        // MissionPoints
        QWebElement points = document.findFirst("#zoneChangeCosts");
        if(!points.isNull()) {
            int ist = state("points").toInt();
            int soll = points.toPlainText().trimmed().toInt();
            if(ist != soll) {
                setState("points", soll);
                adjustTimer();
            }
        }

        QWebElement rewardBox = document.findFirst("#sbox-overlay");
        if(rewardBox.isNull()) return;
        if(rewardBox.styleProperty("display", QWebElement::ComputedStyle) == "block") {
            setState("state", get_reward);
            // einmal klicken:
            // <a class="button"><span>Schließen</span></a>
        } else {
            setState("state", unknown);
        }
        //qDebug() << "rewardBox.style(display) =" <<  rewardBox.styleProperty("display", QWebElement::ComputedStyle);
        // wenn sie an ist, dann mal im Inventory schauen.
    }

    //if(!hasPath(dpath)) return;
    //qDebug() << QString("bk" + name() + "Module::parse (" + dpath + ")").toLocal8Bit().data() << this;
}

void bkMissionsModule::logic(QWebPage* page)
{
    QString dpath = page->mainFrame()->url().path().toLower();

    QList<bkItem*> clues;
    QMapIterator<int, bkItem*> i(p_account->world()->itemsFor(p_account->player()->id()));
    while(i.hasNext()) {
        i.next();
        bkItem* item = i.value();
        if(item->contains("clue_status") && item->value("clue_status").toString() != "Used") {
            if(item->value("clue_status").toString() != "Mission") {
                qDebug() << "clue_item (Reward) in:"
                         << p_account->world()->locationName(item->value("clue_location").toString())
                         << p_account->status("location").toString() << item;
            }
            clues.append(item);
        }
    }

    //if(!hasPath(dpath)) return;
    qDebug() << clues.count() << "offene zettel.";
}

void bkMissionsModule::action(QWebPage*)
{
    // POST:'chooseMission=BurningVillage&missionArt=small&missionKarma=Evil&buyRubies=0'
}

void bkMissionsModule::clickJourney()
{
    QWebPage* page = p_account->currentPage();
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement knob = doc.findFirst("#navJourney");
    if(!knob.isNull()) knob.evaluateJavaScript("this.click();");
}

void bkMissionsModule::view(QWebPage* page)
{
    bkModule::view(page);
    //if(!state("enable").toBool()) return;
    QWebElement moduleElement = getModule(page->mainFrame()->documentElement());

    QWebElement text = moduleElement.findFirst("#kmMissionPoints");
    if(!text.isNull()) return;
    QString location = p_account->status("location").toString();
    QString content;
    text = appendLine(moduleElement);
    if(!location.isNull()) {
        content = p_account->world()->locationId(location).value("title").toString();
    }
    QVariant result = state("points").toVariant();
    if(result.isValid()) {
        content.append(QString(" (<span>%1</span>)").arg(result.toInt()));
        QString script = "var MPUpdate = function() {\n"
                         "  var text = parseInt(account.state('Missions','points'));\n"
                         "  document.id('kmMissionPoints').getElement('span').set('text', text);\n"
                         "}\n";
                         //"MPUpdate();\n";
        page->mainFrame()->evaluateJavaScript(script);
        // << script.toLocal8Bit().data();
    }
    text.appendInside(content);
    text.setAttribute("id", "kmMissionPoints");
    text.setAttribute("align", "right");

    //qDebug() << QString("bk"+name()+"Module::view").toLocal8Bit().data() << this;
}
