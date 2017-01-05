#include "account.h"
#include "battleknight.h"

#include <QUrl>
#include <QUrlQuery>
#include <QDateTime>
#include <QTimerEvent>
#include <QWebFrame>
#include <QWebElement>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QDebug>


Account::Account(const QString cookie, const QUrl url, QObject *parent) :
    QObject(parent),
    m_cookieValue(cookie),
    s_networkManager(0),
    m_cleanTimer(0),
    s_playerManager(new PlayerManager),
    s_itemManager(new ItemManager),
    s_reportManager(new ReportManager)
{
    m_player.insert("missionPoints",QVariant(0));
    m_player.insert("gmPoints",QVariant(0));
    m_accPlayer = 0;

    QStringList parts = url.host().split(".");
    m_accHost = parts.at(0) + "." + parts.at(1);
    //m_accStatus.insert("cookieValue", QJsonValue(m_cookieValue));

    m_missionsTimer = startTimer(60 * 60 * 1000);
    m_gmTimer = startTimer(60 * 60 * 1000);
    connect(s_reportManager, SIGNAL(playerCheck(QVariant)),
            this, SLOT(setPlayer(QVariant)));
}

void Account::toggle(const QString option, const bool soll)
{
    bool ist = isActive(option);

    if(option == "enableAccount") {
        if(soll) {
        } else {
        }
    }

    if(ist != soll) {
        m_accStatus.insert(option,QJsonValue(soll));
        qDebug() << parent()->metaObject()->className() << "Account::toggle:" << option << soll << m_accStatus.value(option).toVariant();
    }
}

void Account::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_cleanTimer) {
        killTimer(m_cleanTimer);
        m_cleanTimer = 0;
        reasonCleaner();
    } else if(event->timerId() == m_missionsTimer) {
        // adjust missionPoints
        if(m_player.contains("gmPoints")) {
            int p = m_player.value("missionPoints").toInt();
            if(m_player.value("premium").toBool()) {
                p += 10;
                if(p < 245) m_player.insert("missionPoints", p);
            } else {
                p += 5;
                if(p < 125) m_player.insert("missionPoints", p);
            }
            //qDebug() << "\n\tadjust missionPoints:" << m_player.value("missionPoints").toInt();
        }
    } else if(event->timerId() == m_gmTimer) {
        // adjust gmPoints
        if(m_player.contains("gmPoints")) {
            int p = m_player.value("gmPoints").toInt() + 5;
            if(p < 125) m_player.insert("gmPoints", p);
            //qDebug() << "\n\tadjust gmPoints:" << m_player.value("gmPoints").toInt();
        }
    }
}

QString Account::fingerprint() const
{
    QString ret;
    if(m_accPlayer > 0 && m_accHost != "") ret = QString(".%1").arg(m_accPlayer).prepend(m_accHost);
    return(ret);
}

QJsonObject Account::state()
{
    QDateTime now = QDateTime::currentDateTime();
    m_accStatus.insert("timestamp",QJsonValue(QString("%1").arg(now.toTime_t())));
    //QJsonDocument debug(m_accStatus);
    //qDebug() << "Account::state:\n" << debug.toJson().constData();
    return(QJsonObject(m_accStatus));
}

void Account::restoreState(const QJsonObject& state)
{
/*
    QJsonDocument old(m_accStatus);
    qDebug() << "Account::restoreState (from):" << old.toJson();
    QJsonDocument debug(state);
    qDebug() << "Account::restoreState (to):" << debug.toJson();
*/
    foreach(QString key, state.keys()) {
        // ältere cooldowns entfernen.
        if(key.startsWith("timer")) {
            if(key.endsWith("EST")) {
            }
        }
        m_accStatus.insert(key,state.value(key));
    }

    QJsonDocument debug(m_accStatus);
    qDebug() << "Account::restoreState (to):" << debug.toJson().constData();
}

void Account::reasonCleaner()
{
    m_player.remove("waitDuration");
    m_player.remove("waitReason");
    m_player.remove("waitTime");
}

void Account::setProfile(const QVariant data)
{
    s_playerManager->checkPlayer(data);

    QJsonDocument json = QJsonDocument::fromVariant(data);
    QJsonObject o = json.object();

    if(o.contains("knight_id") && m_accPlayer == 0) {
        m_accPlayer = o.value("knight_id").toInt();
        // from now on we have an fingerprint...
        emit(playerFound());
    }

    foreach(QString key, o.keys()) {
        // adjust m_gmTimer
        if(key == "gmPoints" && m_player.contains("gmPoints")) {
            if(o.value(key).toVariant() != m_player.value(key)) {
                // adjust m_gmTimer
                killTimer(m_gmTimer);
                qDebug() << "\tadjust m_gmTimer...";
                m_gmTimer = startTimer(60 * 60 * 1000);
            }
        } else if(key == "missionPoints" && m_player.contains("missionPoints")) {
            if(o.value(key).toVariant() != m_player.value(key)) {
                // adjust m_missionsTimer
                killTimer(m_missionsTimer);
                qDebug() << "\tadjust m_missionsTimer...";
                m_missionsTimer = startTimer(60 * 60 * 1000);
            }
        }
        m_player.insert(key,o.value(key).toVariant());
    }

    if(o.contains("waitReason") && m_cleanTimer == 0) {
        m_cleanTimer = startTimer(m_player.value("waitTime").toInt()*1000+150);
    }

    //qDebug() << parent()->metaObject()->className() << "Account::setProfile" << json.toJson();
}

void Account::setStatus(const QVariant data)
{
    QJsonDocument json = QJsonDocument::fromVariant(data);
    QJsonObject o = json.object();
    QDateTime now = QDateTime::currentDateTime();

    if(o.contains("waitReason") && m_cleanTimer == 0) {
        QString reason = o.value("waitReason").toString();
        if(!m_accStatus.contains("timerAccountEST")) {
        }
        //m_cleanTimer = startTimer(m_player.value("waitTime").toInt()*1000+100);
    }

    if(o.contains("module")) {
        QString module = o.value("module").toString();
        if(module == "treasury") {
            if(o.contains("waitTime")) {
                if(o.value("waitTime").toInt() != 0) {
                    m_accStatus.insert("timerTreasuryEST", QJsonValue((qint64) now.addSecs(o.value("waitTime").toInt()).toTime_t()));
                    m_accStatus.insert("timerTreasuryDuration", QJsonValue((qint64) 21600));
                } else {
                    m_accStatus.remove("timerTreasuryEST");
                    m_accStatus.remove("timerTreasuryDuration");
                }
            }
        }
    }

    qDebug() << parent()->metaObject()->className() << "Account::setStatus" << json.toJson().constData();
}

QVariant Account::status(const QString key) const
{
    QVariant ret;

    if(key.startsWith("timer")) {
        QDateTime now = QDateTime::currentDateTime();
        if(key == "timerTreasury") {
            QDateTime est = QDateTime::fromTime_t(m_accStatus.value("timerTreasuryEST").toInt());
            if(est > now) {
                return(QVariant(est.toTime_t() - now.toTime_t()));
            } else {
                if(m_accStatus.contains("timerTreasuryEST")) m_accStatus.remove("timerTreasuryEST");
                if(m_accStatus.contains("timerTreasuryDuration")) m_accStatus.remove("timerTreasuryDuration");
            }
        }
    }

    if(m_accStatus.contains(key)) {
        ret = m_accStatus.value(key).toVariant();
    }

    return(ret);
}

QString Account::profile(const QString key) const
{
    QString ret;

    if(m_player.contains(key)) {
        ret = m_player.value(key).toString();
    }

    return(ret);
}

QString Account::player(const QString id, const QString key) const
{
    QString ret;

    QVariantMap data = s_playerManager->playerData(id.toInt()).toMap();
    if(data.contains(key)) {
        ret = data.value(key).toString();
    }

    return(ret);
}

QString Account::reports(const int count, const QString type) const
{
    QVariant m = s_reportManager->lastReport(count, type);
    QJsonDocument ret = QJsonDocument::fromVariant(m);
    return(ret.toJson());
}

void Account::loadFinished(QWebPage* page)
{
    QWebFrame* mainFrame = page->mainFrame();
    QUrl url = mainFrame->url();
    QStringList paths = url.path().split("/",QString::SkipEmptyParts);

    if(!paths.count()) return; // nothing to do, login evtl?

    if(!s_networkManager) s_networkManager = page->networkAccessManager();

    BattleKnight* plugin = qobject_cast<BattleKnight *>(parent());
    if(plugin) {
        QString script;
        plugin->readDataFile("locations.json",script);
        mainFrame->evaluateJavaScript(script.prepend("var km_locations=").append(";"));
        plugin->readDataFile("checkscript.js",script);
        mainFrame->evaluateJavaScript(script);
    } else {
        return;
    }

    if(paths.at(0) == QString("user")) {
        if(paths.count() == 1) {
            // read inventory
            // put it in the game-script!
            QWebElementCollection items = mainFrame->findAllElements(".inventoryItem");
            foreach(QWebElement item, items) {
                mainFrame->evaluateJavaScript("km_profile.getItemData('"+item.attribute("id")+"');");
            }
        }
    } else if(paths.at(0) == QString("world")) {
        if(paths.count() == 1) {
        } else {
            if(paths.at(1) == QString("location")) {
            } else if(paths.at(1) == QString("travel")) {
            }
        }
    } else if(paths.at(0) == QString("groupmission")) {
/*
 inGroup: <form id="formFound" action="http://s12.de.battleknight.gameforge.com/groupmission/group" method="post">
*/
        if(paths.count() == 1) {
        } else {
            if(paths.at(1) == QString("group")) {
            }
        }
    } else if(paths.at(0) == QString("market")) {
/*
<div id="mainContent" class="marketplace work_singledout">
    <div id="marketplaceContent" class="work">
        <div class="scrollLongTall">
            <div class="cap"></div>
            <div class="innerContent clearfix">
                <p>Du hast während deiner Arbeitszeit Folgendes verdient:</p>
                <table id="workPaycheckTable">
                    <tbody><tr>
                    <td>
                        <div class="workSilverIconPaycheck"></div><em>194</em>
                    </td>
                                        <td>
                        <div id="karmaIcon" class="workKarmaIconPaycheck iconKarmaEvil"></div><em>1</em>
                    </td>
                                        </tr>
                </tbody></table>
                <p><a id="encashLink" class="button"><span>Sold abholen</span></a></p>
            </div>
            <div class="foot"></div>
        </div>
        <form id="formEncash" method="POST" action="http://s12.de.battleknight.gameforge.com/market/work">
            <input type="hidden" name="paycheck" value="encash">
        </form>
    </div>
</div>
*/
        if(paths.count() == 1) {
        } else {
            if(paths.at(1) == QString("work")) {
            } else if(paths.at(1) == QString("merchant")) {
            }
        }
    } else if(paths.at(0) == QString("duel")) {
        if(paths.count() == 2 && paths.at(1) == QString("compare")) {
            //qDebug() << url.query() << m_player.value("contentTitle");
            // Gegner vergleichen
        } else if(paths.count() == 2 && paths.at(1) == QString("duel")) {
            QUrlQuery query(url);
            if(query.hasQueryItem("enemyID")) {
                if(!m_player.contains("waitReason") && m_player.value("contentTitle").toString() == QString("Duell")) {
                    m_player.insert("waitReason",QVariant("duel"));
                    m_cleanTimer = startTimer(m_player.value("waitTime").toInt()*1000+100);
                    // Kampfbericht parsen (zeitversetzt!)
                }
            }
        }
    } else if(paths.at(0) == QString("clan")) {
    } else if(paths.at(0) == QString("manor")) {
    }
}

void Account::replyFinished(QNetworkReply* reply)
{
    QUrl url = reply->url();
    QStringList paths = url.path().split("/",QString::SkipEmptyParts);

    if(!paths.count()) return; // nothing to do, login evtl?

    if(!s_networkManager) s_networkManager = reply->manager();

    if(paths.at(0) == QString("ajax")) {
        if(paths.count() < 3) return;
        if(paths.at(1) == QString("ajax")) {
            if(paths.at(2) == QString("getInventory")) {
                QJsonDocument json = QJsonDocument::fromJson(reply->property("getData").toByteArray());
                QJsonArray items = json.object().value("items").toArray();
                for(int i = 0; i < items.size(); ++i) {
                    setItem(items[i].toObject());
                }
            } else if(paths.at(2) == QString("buyItem")) {
/*

buyItem: "{"result":true,"reason":"","data":{"item":{"item_id":4785682,"item_level":"1",
    "item_info":{"posX":2,"posY":3},"item_width":"1","item_depth":"1","item_inventory":"1","item_own":"28545","item_pic":"Clue01_closed","item_set":0,"item_add":0,
    "item_name":"item\/clue1","item_fullName":"Allt&auml;glicher Hinweis","item_magic_level":"white","item_value":88,"item_ruby":0,"item_slot":11,
    "item_rune":0,"item_rune1":0,"item_rune2":0,"item_rune3":0,"item_str":0,"item_dex":0,"item_end":0,"item_luck":0,"item_weapon":0,"item_shield":0,
    "item_ride":0,"item_critical":0,"item_speed":0,"item_off":0,"item_def":0,"item_damage":0,"item_damage2":0,"item_armour":0,"item_fire":0,"item_ice":0,
    "item_shock":0,"item_poison":0,"item_fire_res":0,"item_ice_res":0,"item_shock_res":0,"item_poison_res":0,"item_special_ability":"a:0:{}",
    "item_manor_seconds":0,"item_desckey":0,"item_location":"FortressTwo","item_additionals":false,
    "item_use":0,
    "clue_item":"4785682","clue_level":"1","clue_status":"Mission","clue_data":{
        "location":"CapitalCity","mission":"MysticalTower"}
    },"silver":17531,"rubies":191,
    "inventory":"01100000000011110100111100101111000011110001111110111111",
    "tooltip_normal":"<div class=\"itemToolTip\"><table width=\"180\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" summary=\"Summary\">  <colgroup><col class=\"itemColumn\" \/>    <col class=\"itemColumn value\" \/><\/colgroup>  <tr>    <th class=\"itemTitle white\" colspan=\"2\" scope=\"row\">    Allt&auml;glicher Hinweis<\/th>  <\/tr>      <tr class=\"tall\">    <th scope=\"row\"><span class=\"icon iconSilver\"><\/span><\/th>    <td scope=\"row\">88<\/td>  <\/tr> <tr class=\"tall\"><th colspan=\"2\" scope=\"row\">Reise nach Endalain. Bei einer der Missionen dort wirst du etwas N&uuml;tzliches finden.<\/th><\/tr>  <tr class=\"short\">    <th scope=\"row\">&nbsp;<\/th>    <td scope=\"row\"><\/td>  <\/tr><tr class=\"tall\"><th scope=\"row\" colspan=\"2\"><\/th><\/tr><\/table><\/div>","tooltip_compare":"<div class=\"itemToolTip\"><table width=\"350\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" summary=\"Summary\"><colgroup><col class=\"itemColumn\" \/>    <col class=\"itemColumn value\" \/>    <col class=\"compareColumn\" \/><col class=\"itemColumn\" \/>    <col class=\"itemColumn value\" \/><\/colgroup>  <tr class=\"tableFirstRow\"><th width=\"44%\" class=\"itemTitle tableCol1 white\" colspan=\"2\" scope=\"row\">Allt&auml;glicher Hinweis<\/th><td width=\"12%\" scope=\"row\" class=\"tableCol2\"><\/td><td width=\"44%\" colspan=\"2\" scope=\"row\" class=\"tableCol3\"><span class=\"itemTitle ##compareClassName##\">##compareName##<\/span>(getragen)<\/td><\/tr><tr class=\"tall\"><th scope=\"row\"><span class=\"icon iconSilver\"><\/span><\/th><td scope=\"row\">88<\/td><td scope=\"row\"><\/td>##compareSilverHalfRow##<\/tr>##rubyRow####runeRow##  <tr class=\"short\">    <th scope=\"row\">&nbsp;<\/th>    <td scope=\"row\"><\/td><td><\/td>    <th scope=\"row\">&nbsp;<\/th>    <td scope=\"row\"><\/td>  <\/tr>  <tr>    <th scope=\"row\">&nbsp;<\/th>    <td scope=\"row\"><\/td>    <td><span class=\"info\">+\/-<\/span><\/td>    <th scope=\"row\">&nbsp;<\/th>    <td scope=\"row\"><\/td>  <\/tr>##compareAttributeRows##<tr class=\"short\"><th scope=\"row\" colspan=\"2\"><\/th><td><\/td><th scope=\"row\" colspan=\"2\">##SpecialAbilities##<\/th><\/tr><\/table><\/div>"
    }
}"

*/
                //qDebug() << "buyItem:" << reply->property("getData").toByteArray();
            } else if(paths.at(2) == QString("sellItem")) {
                qDebug() << "sellItem:" << reply->property("getData").toByteArray();
            } else if(paths.at(2) == QString("placeItem")) {
                qDebug() << "placeItem:" << reply->property("getData").toByteArray();
            } else if(paths.at(2) == QString("wearItem")) {
                qDebug() << "wearItem:" << reply->property("getData").toByteArray();
            }
        } else if(paths.at(1) == QString("duel")) {
            if(paths.at(2) == QString("proposals")) {
                QJsonDocument json = QJsonDocument::fromJson(reply->property("getData").toByteArray());
                if(json.object().value("result").toBool()) {
                    QJsonArray data = json.object().value("data").toArray();
                    QJsonDocument doc(data.at(0).toObject());
                    s_playerManager->checkPlayer(doc.toVariant());
                }
            }
        } else if(paths.at(1) == QString("user")) {
            if(paths.at(2) == QString("changeZones")) {
                // paths.at(3)
                // {"hit1":"2","hit2":"7","hit3":"4","hit4":"4","hit5":"9","shield1":"2","shield2":"5","shield3":"4","shield4":"5","shield5":"4"}
            }
        }
    } else if(paths.at(0) == QString("clanwar")) {
        if(paths.count() > 1 && paths.at(1) == QString("battle")) {
            qDebug() << "clanwar/battle:" << reply->property("getData").toByteArray();
        }
    } else if(paths.at(0) == QString("world")) {
        if(paths.count() > 1 && paths.at(1) == QString("location")) {
            // adjust missionPoints
            QUrlQuery query(reply->property("postData").toString());
            if(query.hasQueryItem("missionArt")) {
                int r = query.queryItemValue("buyRubies").toInt();
                int p = m_player.value("missionPoints").toInt();
                if(query.queryItemValue("missionArt") == QString("small") && r == 0) {
                    p -= 20;
                } else if(query.queryItemValue("missionArt") == QString("medium") && r == 0) {
                    p -= 40;
                } else if(query.queryItemValue("missionArt") == QString("large") && r == 0) {
                    p -= 60;
                }
                if(p >= 0) m_player.insert("missionPoints",QVariant(p));
            }
        } else if(paths.count() > 1 && paths.at(1) == QString("startTravel")) {
            // (/world/startTravel), POST:'travelwhere=CoastalFortressOne&travelhow=horse&travelpremium=0'
            QUrlQuery query(reply->property("postData").toString());
            if(query.hasQueryItem("travelwhere")) {
            }
        }
    } else if(paths.at(0) == QString("mail")) {
        if(paths.count() == 2 && paths.at(1) == QString("getInbox")) {
            QUrlQuery query(reply->property("postData").toString());
            if(query.hasQueryItem("inboxtype") && query.queryItemValue("inboxtype") == "reports") {
                QJsonDocument json = QJsonDocument::fromJson(reply->property("getData").toByteArray());
                QJsonArray mails = json.object().value("mails").toArray();
                //qDebug() << "lese" << mails.count() << "mails.";// << json.toJson(QJsonDocument::Compact);
                while(!mails.isEmpty()) {
                    s_reportManager->checkReport(mails.last().toVariant());
                    mails.removeLast();
                }
            }
        }
    }
}
