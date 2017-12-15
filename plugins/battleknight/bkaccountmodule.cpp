#include "bkaccountmodule.h"
#include "bkaccount.h"
#include "bkplayer.h"

#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QNetworkReply>
#include <QUrlQuery>

#include <QDebug>



bkAccountModule::bkAccountModule(QJsonObject& defaults, bkAccount* account, QObject *parent) :
    bkModule("Account", defaults, account, parent)
{
    addPath("/manor/");
}

void bkAccountModule::job(const QString& jobName, const int& mode)
{
    qDebug() << QString("bk" + name() + "Module::job(%1, %2)").arg(jobName).arg(mode).toLocal8Bit().data();
}

void bkAccountModule::getInventory(QNetworkReply* reply)
{
    QByteArray data = reply->property("getData").toByteArray();

    QJsonDocument json = QJsonDocument::fromJson(data);
    if(!json.object().value("result").toBool()) return;

    bkWorld* world = p_account->world();
    QJsonArray items = json.object().value("items").toArray();
    foreach(QJsonValue iv, items) {
        QJsonObject io = iv.toObject();
        bkItem* item = world->item(io.value("item_id").toString().toInt());
        item->setData(io);

        //qDebug() << "item:" << item;
    }

    qDebug() << "bkAccountModule::getInventory()";// << json.toJson().data();
}

void bkAccountModule::buyItem(QNetworkReply* reply)
{
    QByteArray data = reply->property("getData").toByteArray();
    QJsonDocument json = QJsonDocument::fromJson(data);
    if(!json.object().value("result").toBool()) return;

    QJsonObject d = json.object().value("data").toObject();
    QJsonObject i = d.value("item").toObject();
    bkWorld* world = p_account->world();
    bkItem* item = world->item(i.value("item_id").toInt());
    item->setData(i);

    //qDebug() << "\tbkParser::ajaxBuyItem" << item;
    if(i.contains("clue_data")) {
        qDebug() << "bkAccountModule::buyItem(Zettel gekauft)" << QJsonDocument(i.value("clue_data").toObject()).toJson().data();
    }
}

void bkAccountModule::sellItem(QNetworkReply* reply)
{
    QByteArray data = reply->property("getData").toByteArray();
    QJsonDocument json = QJsonDocument::fromJson(data);
    if(!json.object().value("result").toBool()) return;

    qDebug() << "bkAccountModule::sellItem()" << json.toJson().data();
}

void bkAccountModule::placeItem(QNetworkReply* reply)
{
    QByteArray data = reply->property("getData").toByteArray();
    QJsonDocument json = QJsonDocument::fromJson(data);
    if(!json.object().value("result").toBool()) return;

    qDebug() << "bkAccountModule::placeItem()" << json.toJson().data();
}

void bkAccountModule::wearItem(QNetworkReply* reply)
{
    QByteArray data = reply->property("getData").toByteArray();
    QJsonDocument json = QJsonDocument::fromJson(data);
    if(!json.object().value("result").toBool()) return;

    qDebug() << "bkAccountModule::wearItem()" << json.toJson().data();
}

void bkAccountModule::startTravel(QNetworkReply* reply)
{
    QUrlQuery query(reply->property("postData").toString());
    // travelwhere=HarbourOne&travelhow=cog&travelpremium=0
    if(!query.hasQueryItem("travelwhere")) return;
    qDebug() << "bkAccountModule::startTravel()" << query.queryItemValue("travelwhere");
}

void bkAccountModule::replyFinished(QNetworkReply* reply)
{
    QString dpath = reply->url().path();
    // Inventory überprüfen!
    if(dpath == "/ajax/ajax/getInventory/") {
        getInventory(reply);
    } else if(dpath == "/ajax/ajax/buyItem/") {
        buyItem(reply);
    } else if(dpath == "/ajax/ajax/sellItem/") {
        sellItem(reply);
    } else if(dpath == "/ajax/ajax/placeItem/") {
        placeItem(reply);
    } else if(dpath == "/ajax/ajax/wearItem/") {
        wearItem(reply);
    } else if(dpath == "/world/startTravel") {
        startTravel(reply);
    } else if(dpath == "/world/location") {
        QUrlQuery query(reply->property("postData").toString());
        if(!query.hasQueryItem("missionArt")) return;
        //if(query.queryItemValue("buyRubies").toInt() > 0) return;
        int p = 0;
        QString art = query.queryItemValue("missionArt");
        if(art == "small") {
            p = 10;
        } else if(art == "medium") {
            p = 20;
        } else if(art == "large") {
            p = 30;
        }
        if(p <= 0) return;
        setState("timerCooldownDuration", p*60);
    }
}

void bkAccountModule::restoreState(QJsonObject& state)
{
    bkModule::restoreState(state);
}

void bkAccountModule::checkAccount(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement body = doc.findFirst("body");
    QJsonObject knight;
    QWebElement tester;

    if(body.classes().contains("nonPremium")) {
        knight.insert("manor_royalty", false);
    } else {
        knight.insert("manor_royalty", true);
    }

    //qDebug() << "bkAccountModule::checkAccount:" << body.classes();

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

    bkPlayer* p = p_account->world()->player(knight.value("knight_id").toInt());
    p->setData(knight);

    //tester = doc.findFirst("#contentTitle");
    //if(tester.isNull()) return;
    //qDebug() << "bkAccountModule::checkAccount(#contentTitle)" << p;
}

void bkAccountModule::profileHtml(QWebPage* page)
{
    QStringList paths = page->mainFrame()->url().path().split("/", QString::SkipEmptyParts);
    QString number = paths.at(2);

    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement tester = doc.findFirst("#profileDetails");
    if(tester.isNull()) return;
    QJsonObject data;
    bkPlayer* player = p_account->world()->player(number.toInt());
    data.insert("knight_id", number.toInt());

    QWebElementCollection rows = tester.findAll("td");
    data.insert("knight_level", rows.at(0).toPlainText().trimmed().toInt());
    QWebElement link = rows.at(1).firstChild();
    if(!link.isNull()) { // clan
        QUrl url(link.attribute("href"));
        number = url.path().split("/",QString::SkipEmptyParts).at(2);
        bkClan* clan = p_account->world()->clan(number.toInt());
        QString cl = link.toPlainText().trimmed();
        cl = cl.replace("[","").replace("]","");
        QJsonObject cd;
        cd.insert("clan_id", number.toInt());
        cd.insert("clan_tag", cl);
        clan->setData(cd);
        data.insert("clan_id", number.toInt());
        data.insert("clan_rang", rows.at(2).toPlainText().trimmed());
        //qDebug() << clan->toJson().data();
    }
    data.insert("loot_won", rows.at(3).toPlainText().trimmed().toInt());
    data.insert("loot_lose", rows.at(4).toPlainText().trimmed().toInt());
    data.insert("turniere_won", rows.at(5).toPlainText().trimmed().toInt());
    data.insert("fights_won", rows.at(6).toPlainText().trimmed().toInt());
    data.insert("fights_balance", rows.at(7).toPlainText().trimmed().toInt());
    data.insert("fights_lose", rows.at(8).toPlainText().trimmed().toInt());

    tester = doc.findFirst("#profileAttrib");
    if(tester.isNull()) return;
    rows = tester.findAll("td");
    data.insert("strength", rows.at(0).toPlainText().trimmed().toInt());
    data.insert("dexterity", rows.at(1).toPlainText().trimmed().toInt());
    data.insert("endurance", rows.at(2).toPlainText().trimmed().toInt());
    data.insert("luck", rows.at(3).toPlainText().trimmed().toInt());
    data.insert("weapon", rows.at(4).toPlainText().trimmed().toInt());
    data.insert("shield", rows.at(5).toPlainText().trimmed().toInt());
    QString damage = rows.at(6).toPlainText().trimmed();
    data.insert("damage_min", damage.split("-",QString::SkipEmptyParts).at(0).toInt());
    data.insert("damage_max", damage.split("-",QString::SkipEmptyParts).at(1).toInt());
    data.insert("armour", rows.at(7).toPlainText().trimmed().toInt());

    tester = doc.findFirst("#charSlots");
    if(tester.isNull()) return;
    // items in inventory
    QWebElementCollection items = tester.findAll("div.drop");
    foreach(QWebElement it, items) {
        QWebElement teil = it.firstChild();
        QVariant result = page->mainFrame()->evaluateJavaScript("document.id('"+teil.attribute("id")+"').retrieve('tip:text');");
        //qDebug() << it.attribute("id") << ":" << teil.attribute("id");// << result.toByteArray().data();
    }

    player->setData(data);
    qDebug() << player->toJson().data();
}

void bkAccountModule::userHtml(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement tester = doc.findFirst("#itemLayer");
    if(tester.isNull()) return;

    // items in inventory
    QWebElementCollection items = tester.findAll("div");
    foreach(QWebElement it, items) {
        if(!it.classes().contains("drag")) continue;
        QString id = it.attribute("id");
        //qDebug() << it.attribute("item:id");
        QVariant result = page->mainFrame()->evaluateJavaScript("document.id('" + id + "').retrieve('item:id');");
        if(!result.isValid()) continue;
        bkItem* item = p_account->world()->item(result.toInt());
        result = page->mainFrame()->evaluateJavaScript("document.id('" + id + "').retrieve('item:allAttributes');");
        if(!result.isValid()) continue;
        QJsonDocument debug = QJsonDocument::fromVariant(result);
        item->setData(debug.object());
        //qDebug() << "\titem:" << item->id() << it.attribute("item:allAttributes");// << debug.toJson().data();
    }
}

void bkAccountModule::userKarmaHtml(QWebPage* page, QVariant progressbarEndTime, QVariant progressbarDuration)
{
    if(!progressbarEndTime.isValid() || !progressbarDuration.isValid()) return;
    setState("timerKarma", progressbarEndTime.toInt());
    setState("timerKarmaDuration", progressbarDuration.toInt());
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement symbol = doc.findFirst("#userKarmaSymbol");
    if(symbol.isNull()) return;
    QString skill = symbol.classes().last();
    QVariant result = skill.split("_",QString::SkipEmptyParts).last();
    setState("modusKarma", result.toInt());
}

void bkAccountModule::worldTravelHtml(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();
    // <div id="mainContent" class="map world18">
    QWebElement main = doc.findFirst("#mainContent");

    QWebElementCollection towns = main.findAll("div.askpoint");
    foreach(QWebElement town, towns) {
        QWebElement tName = town.findFirst("div.cap");
        QString out = tName.toPlainText().trimmed();
        if(town.hasAttribute("onmouseover")) out.append(": "+town.attribute("onmouseover"));
        //qDebug() << "bkAccountModule::worldTravelHtml()" << out;
    }
}

void bkAccountModule::worldMapHtml(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();
    // <div id="mainContent" class="map world18">
    QWebElement main = doc.findFirst("#mainContent");
/*
div id="mainContent" class="map world18">
    <div id="mapBase">
        <div id="DragonIcon" class="TradingPostFour"></div>
*/
    QWebElement events = doc.findFirst("#DragonIcon");
    if(!events.isNull()) {
        qDebug() << "bkAccountModule::worldMapHtml(DragonEvent)" << events.classes();
    } else { // anderes Event?
    }

    QWebElementCollection towns = main.findAll("div.askpoint");
    foreach(QWebElement town, towns) {
        QWebElement tName = town.findFirst("div.cap");
        QString out = tName.toPlainText().trimmed();
        //qDebug() << out;
    }
}

void bkAccountModule::manorHtml(QWebPage* page)
{
    QJsonObject knight;
    QWebElement doc = page->mainFrame()->documentElement();
    // <div id="mainContent" class="manorPremium">
    QWebElement main = doc.findFirst("#mainContent");
    QWebElementCollection items = main.findAll("li");
    foreach(QWebElement item, items) {
        QString out = item.attribute("id");
        QWebElement days = item.findFirst("span.manorDays");
        QString manor = "manor_royalty";
        if(out == "manorItem1") {
            manor = "manor_royalty";
        } else if(out == "manorItem2") {
            manor = "manor_guards";
        } else if(out == "manorItem3") {
            manor = "manor_trainer";
        } else if(out == "manorItem4") {
            manor = "manor_treasurer";
        } else if(out == "manorItem5") {
            manor = "manor_comrade";
        } else if(out == "manorItem6") {
            manor = "manor_healer";
        }

        out.append(", " + manor);

        if(!days.isNull()) {
            knight.insert(manor, true);
            setState(item.attribute("id"), days.toPlainText().trimmed().toInt());
            out.append(": " + days.toPlainText().trimmed());
        } else {
            knight.insert(manor, false);
        }
        //qDebug() << out;
    }
    //bkPlayer* player = p_account->player();
    p_account->player()->setData(knight);
    //qDebug() << player;
}

void bkAccountModule::parse(QWebPage* page, QVariant titleTimer, QVariant progressbarEndTime, QVariant progressbarDuration)
{
    QString dpath = page->mainFrame()->url().path().toLower();

    // erst mal die Info-Seiten lesen
    checkAccount(page);
    if(dpath.startsWith("/common/profile/")) {
    } else if(dpath == "/user/") {
        // Attribute und Inventar
        userHtml(page);
    } else if(dpath == "/user/karma/") {
        // Karmakräfte
        userKarmaHtml(page, progressbarEndTime, progressbarDuration);
    } else if(dpath == "/user/zones/") {
        // ToDo:
        // zones
    } else if(dpath == "/user/loot/") {
        // ToDo:
        // loot
    } else if(dpath == "/world/travel") {
        if(!titleTimer.isValid()) worldTravelHtml(page);
    } else if(dpath == "/world/map") {
        worldMapHtml(page);
    } else if(dpath == "/manor/") {
        manorHtml(page);
    }

    if(titleTimer.isValid()) {
        setState("timerCooldown", titleTimer.toInt()+2);
    }
    QWebElement tester = page->mainFrame()->findFirstElement("#mainContent");
    if(!tester.isNull()) {
        QVariant modus(cd_none);
        if(tester.classes().contains("cooldownWork")) {
            modus = cd_work;
            setState("moduleCooldown", "Work");
        } else if(tester.classes().contains("cooldownDuel")) {
            modus = cd_duel;
            setState("moduleCooldown", "Duels");
        } else if(tester.classes().contains("cooldownFight")) {
            modus = cd_fight;
            setState("moduleCooldown", "Mission");
        } else if(tester.classes().contains("cooldownTravel")) {
            modus = cd_travel;
            setState("moduleCooldown", "Travel");
        } else if(tester.classes().contains("fightResult")) { // ("fightResult", "duelFightResult")
            if(tester.classes().contains("duelFightResult")) {
                modus = cd_duel;
                setState("moduleCooldown", "Duels");
            } else {
                modus = cd_fight;
                setState("moduleCooldown", "Mission");
            }
        }
        if(modus != cd_none) {
            setState("modusCooldown", modus.toInt());
            if(progressbarEndTime.isValid()) setState("timerCooldown", progressbarEndTime.toInt() + 2);
            if(progressbarDuration.isValid()) setState("timerCooldownDuration", progressbarDuration.toInt() + 1);
        }
    }
    if(active()) emit jobDone();
}

void bkAccountModule::logic(QWebPage*)
{
    int strength = state("strength").toInt();
    ++strength;
    setState("strength", strength);
}

void bkAccountModule::action(QWebPage*)
{
    //qDebug() << "bkAccountModule::action" << name();
}

void bkAccountModule::view(QWebPage* page)
{
    bkModule::view(page);
    //if(!state("enable").toBool()) return;
    QWebElement moduleElement = getModule(page->mainFrame()->documentElement());

    QWebElement body = page->mainFrame()->findFirstElement("body");
    if(body.classes().contains("nonPremium")) {
        QWebElement netBar = page->mainFrame()->findFirstElement("#mmonetbar");
        netBar.removeFromDocument();
        netBar = page->mainFrame()->findFirstElement("#networkBar");
        netBar.removeFromDocument();
        QWebElement div = body.findFirst("div");
        div.setStyleProperty("position","relative");
        div.setStyleProperty("top","-32px");
        QWebElement head = page->mainFrame()->findFirstElement("head");
        div = head.findFirst("style");
        div.removeFromDocument();
    }

    QWebElement knightName = moduleElement.findFirst("#kmKnightName");
    knightName.setPlainText(p_account->player()->value("knight_name").toString());
    knightName.setAttribute("align", "right");

    QVariant result;

    result = state("timerCooldown");
    if(result.isValid() && result.toInt() > 1) {
        int modus = state("modusCooldown").toInt();
        QString m = "unbekannt";
        switch(modus) {
            case cd_work:
                m = "Arbeiten";
                break;
            case cd_duel:
                m = "Duell";
                break;
            case cd_fight:
                m = "Mission";
                break;
            case cd_travel:
                m = "Reise";
                break;
            default:
                break;
        }
        addProgressBar(moduleElement, "Cooldown", m);
    }

    result = state("timerKarma");
    if(result.isValid() && result.toInt() > 0) {
        addProgressBar(moduleElement, "Karma", "Karma");
        QWebElement progressMeter = moduleElement.findFirst("#timeMeterAccountKarma");
        progressMeter.setStyleProperty("background-color", "rgba(0, 255, 0, 0.35)");
/*
        var dir = 'up';
        if(typeof document.id('timeMeter'+topic).get('km_direction') !== 'undefined') {
            dir = document.id('timeMeter'+topic).get('km_direction');
        }
*/
        progressMeter.addClass("timeMeterDown");
    }

    result = state("timerLogic");
    if(result.isValid() && result.toInt() > 0) {
        int modus = state("modusLogic").toInt();
        QString m = "unknown";
        switch(modus) {
            case w_nothing:
                m = "relax";
                break;
            case w_call_working:
                m = "call working";
                break;
            case w_go_working:
                m = "go working";
                break;
            case w_get_paying:
                m = "get paying";
                break;
            case w_call_gm:
                m = "call gm";
                break;
            case w_reload_gm:
                m = "reload gm";
                break;
            case w_call_duel:
                m = "call duell";
                break;
            case w_check_proposal:
                m = "check proposal";
                break;
            case w_next_proposal:
                m = "next proposal";
                break;
            case w_go_compare:
                m = "go compare";
                break;
            case w_go_duel:
                m = "go duel";
                break;
            default:
                break;
        }

        addProgressBar(moduleElement, "Logic", m);
        QWebElement progressMeter = moduleElement.findFirst("#timeMeterAccountLogic");
        progressMeter.setStyleProperty("background-color", "rgba(255, 255, 0, 0.30)");
        progressMeter.setAttribute("km_direction", "down");
    }
    //qDebug() << QString("bk"+name()+"Module::view").toLocal8Bit().data() << this;
}
