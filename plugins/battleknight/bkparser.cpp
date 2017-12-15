#include "bkparser.h"
#include "bkaccount.h"
#include "bkitem.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMimeDatabase>
#include <QUrl>
#include <QWebFrame>
#include <QWebElementCollection>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QTimer>
#include <QProcess>

#include <QDebug>



bkParser::bkParser(QObject *parent) :
    QObject(parent),
    p_page(new QWebPage),
    p_networkManager(Q_NULLPTR)
{
    p_account = qobject_cast<bkAccount*>(parent);
    Q_ASSERT(p_account);
    //qDebug() << "bkParser created.";
}

bkParser::~bkParser()
{
    p_page->deleteLater();
    //qDebug() << "bkParser destroyed.";
}

void bkParser::replyFinished(QNetworkReply* reply)
{
    if(!p_networkManager) {
        p_networkManager = reply->manager();
        p_page->setNetworkAccessManager(p_networkManager);
    }

    QStringList paths = reply->url().path().split("/", QString::SkipEmptyParts);
    if(!paths.count()) return; // nothing to do

    QByteArray data = reply->property("getData").toByteArray();

    //if(data.length() == 0) return;
    QMimeDatabase mimeDB;
    QMimeType type = mimeDB.mimeTypeForData(data);
    if(data.length() > 0 && type.name() == "text/html") {
        // this urls raise js-errors!
        if(reply->url().path() == "/groupmission/group/" ||
                reply->url().path() == "/world/location" ||
                reply->url().path() == "/world/travel" ||
                reply->url().path() == "/world" ||
                reply->url().path() == "/mail/reports" ||
                reply->url().path().startsWith("/market/work") ||
                reply->url().path() == "/clanwar/prepare" ||
                reply->url().path() == "/tavern/battle" ||
                reply->url().path().startsWith("/common/orderprofile/"))
        {
            //return;
        } else {
            //p_page->mainFrame()->setContent(data, type.name(), QUrl("http://localhost"));
            //p_page->mainFrame()->evaluateJavaScript("window.onerror = null;");
        }
    }
    checkReplyPaths(reply);

    //qDebug() << "\tbkParser::replyFinished" << data.length() << reply->url();
}

void bkParser::loadFinished(QWebPage* page)
{
    //checkAccount(page);

    QVariant titleTimer = page->mainFrame()->evaluateJavaScript("if(typeof l_titleTimerEndTime !== 'undefined') l_titleTimerEndTime;");
    if(titleTimer.isValid()) {
        titleTimer = titleTimer.toInt() + 1;
        p_account->setStatus("timerCooldown", titleTimer);
    }
    //qDebug() << "bkParser::loadFinished(l_titleTimerEndTime)" << titleTimer;

    QWebElement tester = page->mainFrame()->findFirstElement("#mainContent");
    if(!tester.isNull()) {
        QVariant modus(cd_none);
        if(tester.classes().contains("cooldownWork")) {
            modus = cd_work;
        } else if(tester.classes().contains("cooldownDuel")) {
            modus = cd_duel;
        } else if(tester.classes().contains("cooldownFight")) {
            modus = cd_fight;
        } else if(tester.classes().contains("cooldownTravel")) {
            modus = cd_travel;
        } else if(tester.classes().contains("fightResult")) { // ("fightResult", "duelFightResult")
            if(tester.classes().contains("duelFightResult")) {
                modus = cd_duel;
            } else {
                modus = cd_fight;
            }
        }
        if(tester.classes().contains("cooldownWork") ||
                tester.classes().contains("cooldownDuel") ||
                tester.classes().contains("cooldownFight") ||
                tester.classes().contains("fightResult") ||
                tester.classes().contains("cooldownTravel"))
        {
            p_account->setStatus("modusCooldown", modus);
            QVariant timer = page->mainFrame()->evaluateJavaScript("if(typeof progressbarEndTime !== 'undefined') progressbarEndTime;");
            if(timer.isValid()) {
                timer = timer.toInt() + 1;
                p_account->setStatus("timerCooldown", timer);
            }
            QVariant duration = page->mainFrame()->evaluateJavaScript("if(typeof progressbarDuration !== 'undefined') progressbarDuration;");
            if(duration.isValid()) p_account->setStatus("timerCooldownDuration", duration);
        }
    }

    QStringList paths = page->mainFrame()->url().path().split("/", QString::SkipEmptyParts);
    QString first = paths.takeFirst();

    if(first == "user") {
        if(!paths.isEmpty()) {
            QString second = paths.takeFirst();
            if(second == "karma") {
                userKarmaHtml(page);
            } else if(second == "zones") {// Trefferzonen
            } else if(second == "loot") {// Beutekiste
            }
        } else {
            userHtml(page);
        }
    } else if(first == "common") {
        if(!paths.isEmpty()) {
            QString second = paths.takeFirst();
            if(second == "profile") {
                commonProfileHtml(page);
            }
        }
    } else if(first == "tavern") {
        if(!paths.isEmpty()) {
            QString second = paths.takeFirst();
            if(second == "prepare") {// Kriegsvorbereitung
                //tavernPrepareHtml(page);
            } else if(second == "battle") {// Krieg
                //tavernBattleHtml(page);
            } else if(second == "fight") {// Kämpfe
            }
        }
    } else if(first == "clanwar") {
        if(!paths.isEmpty()) {
            QString second = paths.takeFirst();
            if(second == "prepare") {// Kriegsvorbereitung
                //tavernPrepareHtml(page);
            } else if(second == "battle") {// Krieg
                //tavernBattleHtml(page);
            } else if(second == "fight") {// Kämpfe
            }
        }
    } else if(first == "manor") {// Landsitz
    } else if(first == "treasury") {
        //treasuryHtml(page);
    } else if(first == "world") {
        if(!paths.isEmpty()) {
            QString second = paths.takeFirst();
            if(second == "travel") {// Reisekarte
            } else if(second == "location") {
                worldHtml(page);
            }
        } else { // auf "Welt" geklickt...
            worldHtml(page);
        }
    } else if(first == "joust") {// Turnier
        if(paths.isEmpty()) {
            //joustHtml(page);
        } else {
            QString second = paths.takeFirst();
            if(second == "zones") {// Trefferzonen Turnier
            } else if(second == "tent") {// Turnierzelt
            }
        }
    } else if(first == "market") {
        // work
        // merchant/artefakts
        /*
         *  <div class="merchantBg rings">
         *      <div id="merchItemLayer" class="tabContent merchantDrop">
         *          <div class="itemRing22 itemToolTipEl itemContainer1x1 drag merchantItem" id="merchItem155493477" style="position: absolute; left: 128px; top: 0px;">&nbsp;</div>
         *          <div class="itemRing22 itemToolTipEl itemContainer1x1 drag merchantItem" id="merchItem155493478" style="position: absolute; left: 160px; top: 0px;">&nbsp;</div>
         *          <div class="itemRing06 itemToolTipEl itemContainer1x1 drag merchantItem" id="merchItem155493474" style="position: absolute; left: 32px; top: 0px;">&nbsp;</div>
         *          <div class="itemRing06 itemToolTipEl itemContainer1x1 drag merchantItem" id="merchItem155493476" style="position: absolute; left: 96px; top: 0px;">&nbsp;</div>
         *          <div class="itemRing06 itemToolTipEl itemContainer1x1 drag merchantItem" id="merchItem155493479" style="position: absolute; left: 192px; top: 0px;">&nbsp;</div>
         *          <div class="itemRing24 itemToolTipEl itemContainer1x1 drag merchantItem" id="merchItem155493473" style="position: absolute; left: 0px; top: 0px;">&nbsp;</div>
         *          <div class="itemRing24 itemToolTipEl itemContainer1x1 drag merchantItem" id="merchItem155493475" style="position: absolute; left: 64px; top: 0px;">&nbsp;</div>
         *          <div class="itemRing24 itemToolTipEl itemContainer1x1 drag merchantItem" id="merchItem155493480" style="position: absolute; left: 224px; top: 0px;">&nbsp;</div>
         *          <div class="itemClue02_closed itemToolTipEl itemContainer1x1 drag merchantItem" id="merchItem155493481" style="position: absolute; left: 0px; top: 32px;">&nbsp;</div>
         * 		</div>
         *  </div>
         */
    } else if(first == "duel") {
        if(!paths.isEmpty()) {
            QString second = paths.takeFirst();
            if(second == "compare") { // /duel/compare
                //duelCompareHtml(page);
            } else if(second == "duel") { // /duel/duel
                //duelDuelHtml(page);
            }
        } else { // auf "Turnierplatz" geklickt...
            //qDebug() << "duel - Turnierplatz angeklickt...";
            //if(!titleTimer.isValid()) duelHtml(page);
        }
    } else if(first == "highscore") {
        if(!paths.isEmpty()) { // /highscore/order/
        } else {
            //highscoreHtml(page);
        }
    }
    //qDebug() << "\tbkParser::loadFinished(#mainContent)" << tester.classes();
}

void bkParser::checkReplyPaths(QNetworkReply* reply)
{
    QStringList paths = reply->url().path().split("/", QString::SkipEmptyParts);
    if(!paths.count()) return; // nothing to do
    QByteArray data = reply->property("getData").toByteArray();
    QString first = paths.takeFirst();

    if(first == "userapi") {
        if(!paths.isEmpty()) {
            QString second = paths.takeFirst();
            if(second == "jserror") { // /userapi/jserror/
                qDebug() << "---> checkReplyPaths jserror:" << reply->property("postData").toString() << reply->property("getData").toString() << reply->url().toString();
            }
        }
    } else if(first == "clan") {
        if(!paths.isEmpty()) {
            QString second = paths.takeFirst();
            if(second == "members") { // /clan/members
                /*
                 * <table id="membersTable" class="ordenTable" border="0" cellspacing="0" cellpadding="0">
                 *   <tbody>
                 *     <tr id="recordMember22749">
                 *       <td class="memberActivity">
                 *         <div id="activity22749" class="icon iconActivity defaultCursor mediumToolTip activity05"></div>
                 *       </td>
                 *       <td class="memberRemove"></td>
                 *       <td class="memberRank">Oberhiwi</td>
                 *       <td class="memberName">
                 *         <a id="playerLink" href="https://s12-de.battleknight.gameforge.com:443/common/profile/22749/Order/Members">Vizegraf Baron Greifenstein</a>
                 *       </td>
                 *       <td class="memberLevel">49</td>
                 *       <td class="memberSilver"><div class="icon iconSilverSml"></div>851.586</td>
                 *       <td class="memberRubies"><div class="icon iconRuby"></div>	0</td>
                 *     </tr>
                 *   </tbody>
                 * </table>
                 */
            } else if(second == "externmembers") { // /clan/externmembers/1667
                if(!paths.isEmpty()) {
                    //int clan = paths.takeFirst().toInt();
                }
                // <table id="membersTable"
                // <tbody><tr id="recordMember21754">
                // <td class="memberActivity">
                // <td class="memberRemove">
                // <td class="memberRank">      Konsul						</td>
                // <td class="memberName">
                //    <a id="playerLink" href="https://s12-de.battleknight.gameforge.com:443/common/profile/21754/Scores/Player">DarkenRahl</a>
                // <td class="memberLevel">                 220						</td>
                // <td class="memberSilver">
                // <td class="memberRubies">
            }
        }
    } else if(first == "mail") {
        if(!paths.isEmpty()) {
            QString second = paths.takeFirst();
            if(second == "getInbox") { // /mail/getInbox
                QUrlQuery query(reply->property("postData").toString());
                if(query.hasQueryItem("inboxtype") && query.queryItemValue("inboxtype") == "reports") {
                    //mailInbox(data);
                }
            }
        }
    } else if(first == "ajax") {
        if(!paths.isEmpty()) {
            QString second = paths.takeFirst();
            if(second == "ajax") {
                if(!paths.isEmpty()) {
                    QString third = paths.takeFirst();
                    if(third == "buyItem") { // /ajax/ajax/buyItem
                        //ajaxBuyItem(data);
                    } else if(third == "sellItem") { // /ajax/ajax/sellItem
                    } else if(third == "placeItem") { // /ajax/ajax/placeItem
                        //qDebug() << data.data();
                    } else if(third == "wearItem") { // /ajax/ajax/wearItem
                    } else if(third == "getInventory") { // /ajax/ajax/getInventory
                        //ajaxGetInventory(data);
                    }
                }
            } else if(second == "duel") {
                if(!paths.isEmpty() && paths.at(0) == "proposals") { // /ajax/duel/proposals
                    ajaxProposal(data);
                }
            } else if(second == "user") {
                if(!paths.isEmpty() && paths.at(0) == "changeZones") { // /ajax/user/changeZones
                    //ajaxProposal(data);
                }
            }
        }
    }
    //qDebug() << "\tbkParser::checkReplyPaths" << first << paths;
}

void bkParser::checkAccount(QWebPage* page)
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

    bkPlayer* p = p_account->world()->player(knight.value("knight_id").toInt());
    p->setData(knight);

    tester = doc.findFirst("#contentTitle");
    if(tester.isNull()) return;
    qDebug() << "\tbkParser::checkAccount(#contentTitle)" << tester.toPlainText().trimmed();
    return;
}

/*
 *  Turnier
 *
 *      <div id="joustMembers">
            <h3>Turniereintragung</h3>
            <table class="profileTable">
                <tbody><tr>
                    <th scope="row">Ritter in der Warteliste:</th>
                    <td class="joustQueue mediumToolTip">8 (+2)</td>
                </tr>
                <tr>
                    <th scope="row">Ritter im nächsten Turnier:</th>
                    <td>8</td>
                </tr>
                <tr>
                    <th scope="row">Eigene Wartelistenposition:</th>
                    <td>7</td>
                </tr>
            <!-- end #joustApplyTable-->
            </tbody></table>
            <div id="joustApply">
                <label>Anmeldekosten:</label>
                <div class="formField">
                    <span class="icon iconSilver"></span> 0				</div>
                <div class="commonBtnWrapper submit">
                    <a id="btnApply" class="button disabledBtn mediumToolTip" onclick="applyJoust();"><span>Du bist bereits angemeldet.</span></a>
                </div>
            </div>
        <!-- end #joustMembers-->
        </div>
 */
void bkParser::joustHtml(QWebPage* page)
{
    QWebElement button = page->mainFrame()->findFirstElement("a#btnApply");
    if(!button.isNull()) {
        if(button.hasClass("disabledBtn")) {
            QVariant ret(2);
            p_account->setStatus("modusTurnier", ret);
            qDebug() << "Zum Turnier angemeldet. Warten auf Start.";
        } else {
            QVariant ret(1);
            p_account->setStatus("modusTurnier", ret);
            ret = 0;
            p_account->setStatus("timerTurnier", ret);
            qDebug() << "Zum Turnier anmelden!";
        }
        return;
    }
    // progressbarEndTime / progressbarDuration
    QVariant progressbarEndTime = page->mainFrame()->evaluateJavaScript("if(typeof progressbarEndTime !== 'undefined') progressbarEndTime;");
    QVariant progressbarDuration = page->mainFrame()->evaluateJavaScript("if(typeof progressbarDuration !== 'undefined') progressbarDuration;");
    if(!progressbarEndTime.isValid() || !progressbarDuration.isValid()) return;
    p_account->setStatus("timerTurnier", progressbarEndTime);
    p_account->setStatus("timerTurnierDuration", progressbarDuration);
    // Runde suchen:
    QWebElement main = page->mainFrame()->findFirstElement("div#mainContent");
    QWebElementCollection rounds = main.findAll("div.boardHead");
    if(rounds.count() == 0) return;
    foreach(QWebElement head, rounds) {
        QWebElement h4 = head.firstChild();
        QString begin = h4.firstChild().toPlainText().trimmed();
        if(!begin.contains(" - ")) {
            int runde = head.toPlainText().trimmed().split(" ", QString::SkipEmptyParts).at(1).toInt();
            QVariant ret(runde + 2);
            p_account->setStatus("modusTurnier", ret);
            qDebug() << "bkParser::joustHtml(rounds)" << runde << begin;
        }
    }
}

/*
 *  Schatzkammer
 */
void bkParser::treasuryHtml(QWebPage* page)
{
    QVariant result = page->mainFrame()->evaluateJavaScript("if(typeof progressbarEndTime !== 'undefined') progressbarEndTime;");
    if(result.isValid()) {
        p_account->setStatus("timerTreasury",result);
    } else {
        result = 0;
        p_account->setStatus("timerTreasury",result);
    }
}

/*
 * /common/profile
 */
void bkParser::commonProfileHtml(QWebPage* page)
{
    QString path = page->mainFrame()->url().path();
    int nr = path.split("/",QString::SkipEmptyParts).at(2).toInt();
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement tester = doc.findFirst("#profileDetails");
    if(tester.isNull()) return;
    QJsonObject data;
    bkPlayer* player = p_account->world()->player(nr);
    data.insert("knight_id", nr);
    QWebElementCollection rows = tester.findAll("td");
    data.insert("knight_level", rows.at(0).toPlainText().trimmed().toInt());
    QWebElement link = rows.at(1).firstChild();
    if(!link.isNull()) { // clan
        QUrl url(link.attribute("href"));
        nr = url.path().split("/",QString::SkipEmptyParts).at(2).toInt();
        bkClan* clan = p_account->world()->clan(nr);
        QString cl = link.toPlainText().trimmed();
        cl = cl.replace("[","").replace("]","");
        QJsonObject cd;
        cd.insert("clan_id", nr);
        cd.insert("clan_tag", cl);
        clan->setData(cd);
        data.insert("clan_id", nr);
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

/*
 * /duel/compare
 */
void bkParser::duelCompareHtml(QWebPage* page)
{
    QString path = page->mainFrame()->url().path();
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement tester = doc.findFirst("#profileAttrib");
    if(tester.isNull()) return;
    QJsonObject data;
    bkPlayer* player = p_account->player();
    //data.insert("knight_id", nr);
    QWebElementCollection rows = tester.findAll("td");
    if(rows.count() < 8) return;
    data.insert("strength", rows.at(0).toPlainText().trimmed().toInt());
    data.insert("dexterity", rows.at(1).toPlainText().trimmed().toInt());
    data.insert("endurance", rows.at(2).toPlainText().trimmed().toInt());
    data.insert("luck", rows.at(3).toPlainText().trimmed().toInt());
    data.insert("offensive", rows.at(4).toPlainText().trimmed().toInt());
    data.insert("defensive", rows.at(5).toPlainText().trimmed().toInt());
    QString damage = rows.at(6).toPlainText().trimmed();
    data.insert("damage_min", damage.split("-",QString::SkipEmptyParts).at(0).toInt());
    data.insert("damage_max", damage.split("-",QString::SkipEmptyParts).at(1).toInt());
    data.insert("armour", rows.at(7).toPlainText().trimmed().toInt());

    player->setData(data);

    QUrlQuery query(page->mainFrame()->url().query());
    // enemyID
    int nr = query.queryItemValue("enemyID").toInt();
    player = p_account->player(nr);
    tester = doc.findFirst("#challengerAttrib");
    rows = tester.findAll("td");
    if(rows.count() < 8) return;
    data.insert("strength", rows.at(0).toPlainText().trimmed().toInt());
    data.insert("dexterity", rows.at(1).toPlainText().trimmed().toInt());
    data.insert("endurance", rows.at(2).toPlainText().trimmed().toInt());
    data.insert("luck", rows.at(3).toPlainText().trimmed().toInt());
    data.insert("offensive", rows.at(4).toPlainText().trimmed().toInt());
    data.insert("defensive", rows.at(5).toPlainText().trimmed().toInt());
    damage = rows.at(6).toPlainText().trimmed();
    data.insert("damage_min", damage.split("-",QString::SkipEmptyParts).at(0).toInt());
    data.insert("damage_max", damage.split("-",QString::SkipEmptyParts).at(1).toInt());
    data.insert("armour", rows.at(7).toPlainText().trimmed().toInt());

    player->setData(data);
    //qDebug() << player->toJson().data();
}

// attributes of account-user
void bkParser::userHtml(QWebPage* page)
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

void bkParser::userKarmaHtml(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();
    QVariant progressbarEndTime = page->mainFrame()->evaluateJavaScript("if(typeof progressbarEndTime !== 'undefined') progressbarEndTime;");
    if(!progressbarEndTime.isValid()) return;
    QVariant progressbarDuration = page->mainFrame()->evaluateJavaScript("if(typeof progressbarDuration !== 'undefined') progressbarDuration;");
    if(!progressbarDuration.isValid()) return;
    p_account->setStatus("timerKarma", progressbarEndTime);
    p_account->setStatus("timerKarmaDuration", progressbarDuration);
    // <div id="userKarmaSymbol" class="acidOn mediumToolTip dev_karmaSkill_5">
    QWebElement symbol = doc.findFirst("#userKarmaSymbol");
    if(symbol.isNull()) return;
    QString skill = symbol.classes().last();
    QVariant result = skill.split("_",QString::SkipEmptyParts).last();
    p_account->setStatus("modusKarma", result);
    //qDebug() << result;
}

void bkParser::groupmissionGroupHtml(QWebPage*)
{
}

void bkParser::worldHtml(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement main = doc.findFirst("#mainContent");

    if(main.isNull()) return;
    if(!main.classes().contains("location")) return;

    // Location
    QVariant data(main.classes().last());
    p_account->setStatus("location", data);
    // MissionPoints
    QWebElement points = doc.findFirst("#zoneChangeCosts");
    if(points.isNull()) return;
    data = points.toPlainText().trimmed().toInt();
    p_account->setStatus("missionPoints", data);
}

void bkParser::duelHtml(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();
    /*
     * <div id="proposals">
     *   <div class="formLine clearfix"><label>Name:</label>
ohne:*     <div class="formField"><a href="https://s12-de.battleknight.gameforge.com/common/profile/52805/Tournament/Duel">Faceman</a><span class="light">Stufe 50</span></div>
mit: *     <div class="formField"><a href="https://s12-de.battleknight.gameforge.com/common/profile/4315/Tournament/Duel">wolvarin</a><a href="https://s12-de.battleknight.gameforge.com/common/orderprofile/795/Tournament/Duel">[WoF]</a><span class="light">Stufe 49</span></div>
     *   <!-- end .formLine--></div>
     *   <div class="formLine clearfix">
     *     <div class="formField noLabel">
     *       <div class="commonBtnWrapper floatLeft"><a id="devBtnNext" class="button dev" href="https://s12-de.battleknight.gameforge.com/duel/compare/?enemyID=52805"><span>Weiter</span></a></div>
     *     </div>
     *   <!-- end .formLine--></div>
     * </div>
     *
     * <div class="commonBtnWrapper floatLeft">
     *   <a class="button" onclick="requestProposals('stronger');"><span>Stärker</span></a>
     *   <a class="button" onclick="requestProposals('weaker');"><span>Schwächer</span></a>
     * </div
     */
    QWebElement main = doc.findFirst("#mainContent");
    if(main.isNull()) return;
    QWebElement proposal = doc.findFirst("#proposals").firstChild();
    if(proposal.isNull()) return;
    QWebElementCollection links = proposal.findAll("a");
    if(links.count() == 0) return;
    QWebElement tester = links.at(0);
    QUrl url(tester.attribute("href"));
    /*
     * {
    "clan_id": null,
    "clan_tag": "",
    "knight_id": "53615",
    "knight_level": "47",
    "knight_name": "Lajos"
}
     */
    QJsonObject po;
    po.insert("knight_id", url.path().split("/",QString::SkipEmptyParts).at(2).toInt());
    po.insert("knight_name", tester.toPlainText().trimmed());
    if(links.count() == 2) {
        tester = links.at(1);
        url =tester.attribute("href");
        po.insert("clan_id", url.path().split("/",QString::SkipEmptyParts).at(2).toInt());
        QString tag = tester.toPlainText().trimmed();
        tag.chop(1);
        tag = tag.mid(1);
        po.insert("clan_tag", tag);
    }
    tester = tester.nextSibling();
    QString stufe = tester.toPlainText();
    stufe = stufe.mid(stufe.indexOf(" "));
    po.insert("knight_level", stufe.toInt());

    bkWorld* world = p_account->world();
    bkPlayer* p = world->player(po.value("knight_id").toInt());
    p->setData(po);
    //clanManager
    if(!po.value("clan_id").isNull()) {
        bkClan* clan = world->clan(po.value("clan_id").toInt());
        clan->setData(po);
    }
    QVariant num((int)p->id());
    p_account->setStatus("currentProposal", num);

    //qDebug() << "bkParser::duelHtml(proposal)\n" << p;
             //<< QJsonDocument(po).toJson().data();
}

// /duel/duel
void bkParser::duelDuelHtml(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement main = doc.findFirst("#mainContent");
    if(main.isNull()) return;
    if(!main.classes().contains("duelFightResult")) return;
/*
    QVariant titleTimer = page->mainFrame()->evaluateJavaScript("if(typeof l_titleTimerEndTime !== 'undefined') l_titleTimerEndTime;");
    int wait = qrand() % ((titleTimer.toInt()*1000)/6) + 123;
    qDebug() << "\n" << "---> /duel/duel (titleTimer)" << titleTimer.toInt() << wait;
    QTimer::singleShot(wait, this, SLOT(callReports()));
*/
}

void bkParser::callReports()
{
    QVariant gf_url = p_page->mainFrame()->evaluateJavaScript("if(typeof g_url !== 'undefined') g_url;");
    if(!gf_url.isValid()) return;
    p_page->mainFrame()->load(QUrl(gf_url.toString().append("/mail/reports")));
    qDebug() << "---> bkParser::callReports()" << gf_url.toString().append("/mail/reports");
}

void bkParser::highscoreHtml(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement hsTable = doc.findFirst("#highscoreTable");
    if(hsTable.isNull()) return;
    bkWorld* world = p_account->world();
    QWebElement hsBody = hsTable.findFirst("tbody");
    QWebElementCollection hsRows = hsBody.findAll("tr");
    foreach(QWebElement hsRow, hsRows) {
        if(hsRow.classes().contains("userSeperator")) continue;
        QWebElementCollection hsCols = hsRow.findAll("td");
        QJsonObject knight;
        foreach(QWebElement col, hsCols) {
            if(col.classes().contains("highscore02")) {
                if(col.firstChild().classes().contains("iconKarmaEvil")) knight.insert("knight_course",QJsonValue(false));
                else knight.insert("knight_course",QJsonValue(true));
            }
            if(col.classes().contains("highscore03")) {
                QWebElementCollection link = col.findAll("a");
                QUrl url(link.at(0).attribute("href"));
                QStringList upath = url.path().split("/",QString::SkipEmptyParts);
                qDebug() << upath;
                if(upath.count() == 0) {
                    qDebug() << link.at(1).attribute("href");
                    url = link.at(1).attribute("href");
                    upath = url.path().split("/",QString::SkipEmptyParts);
                    //continue;
                }
                if(upath.at(1) == "profile") {
                    knight.insert("knight_id", QJsonValue(upath.at(2).toInt()));
                    QString name = link.at(0).toPlainText();
                    int pos = name.indexOf(' ');
                    knight.insert("knight_rang", QJsonValue(name.left(pos)));
                    knight.insert("knight_name", QJsonValue(name.mid(pos+1)));
                    if(link.count() > 1) {
                        url = link.at(1).attribute("href");
                        knight.insert("clan_id", QJsonValue(url.path().split("/",QString::SkipEmptyParts).at(2).toInt()));
                        //clanManager
                        bkClan* clan = world->clan(knight.value("clan_id").toInt());
                        QJsonObject co;
                        QString tag = link.at(1).toPlainText().trimmed();
                        tag = tag.mid(1); tag.chop(1);
                        co.insert("clan_id", QJsonValue(url.path().split("/",QString::SkipEmptyParts).at(2).toInt()));
                        co.insert("clan_tag", QJsonValue(tag));
                        clan->setData(co);
                    }
                }
            }
            if(col.classes().contains("highscore04")) {
                knight.insert("knight_level", QJsonValue(col.toPlainText().trimmed().toInt()));
            }
            if(col.classes().contains("highscore05")) {
                knight.insert("loot_won", QJsonValue(col.toPlainText().trimmed().replace(".","").toInt()));
            }
            if(col.classes().contains("highscore06")) {
                knight.insert("fights", QJsonValue(col.toPlainText().trimmed().replace(".","").toInt()));
            }
            if(col.classes().contains("highscore07")) {
                knight.insert("fights_won", QJsonValue(col.toPlainText().trimmed().replace(".","").toInt()));
            }
            if(col.classes().contains("highscore08")) {
                knight.insert("fights_lose", QJsonValue(col.toPlainText().trimmed().replace(".","").toInt()));
            }
        }
        bkPlayer* p = world->player(knight.value("knight_id").toInt());
        p->setData(knight);
    }
}


void bkParser::mailInbox(const QByteArray& data)
{
    QJsonDocument json = QJsonDocument::fromJson(data);
    //qDebug() << "mailInbox:" << data.data();
    if(json.object().value("countMails").toInt() == 0) return;

    bkWorld* world = p_account->world();
    QJsonArray mails = json.object().value("mails").toArray();
    foreach(QJsonValue mv, mails) {
        QJsonObject mo = mv.toObject();
        // mo.keys("knight_id", "mail_content", "mail_html", "mail_id", "mail_receiver", "mail_receiver_status", "mail_sender", "mail_sender_status", "mail_senttime", "mail_subject", "mail_type", "recipient_enemy_defeated", "recipient_gender", "recipient_name")

        //int mailId = mo.value("mail_id").toString().toInt();
        int mailType = mo.value("mail_type").toString().toInt();
        // (mailType == 0) Kampfbericht / Als Söldner angeheuert. / Angriffsphase / Lanzenstechen - Turnierstart
        // (mailType == 2) Ordensnachricht
        // (mailType == 3) Ordensschlacht
        //qDebug() << "mail:" << mailId << mailType << mo.value("mail_senttime").toString() << mo.value("mail_subject").toString();
        if(mailType != 0 && mailType != 3) continue;

        QJsonObject mc = mo.value("mail_content").toObject();
        // Turnierstart () no mail_content
        if(mc.isEmpty()) {
            // war_type(raid)

            // (mailType == 0)
            if(mo.value("mail_subject").toString() == "Als Söldner angeheuert.") {
                // mo.value("mail_senttime").toString() + (24h + 9 x 8h) = clanWar-Ende
            } else if(mo.value("mail_subject").toString() == "Angriffsphase") {
                // Runde 1.
            } else if(mo.value("mail_subject").toString() == "Lanzenstechen - Turnierstart") {
                // Runde 1. in 24h
            }

            qDebug() << "\tbkParser::mailInbox" << mailType << QJsonDocument(mo).toJson().data() << mc;
            continue;
        }
        if(mc.contains("war_id")) {
            // fightType("")
            // Ordensschlacht ("war_attacker", "war_castle", "war_defender", "war_endtime", "war_fight_data", "war_id", "war_initiator", "war_looted_silver", "war_payment_data", "war_result", "war_rounds", "war_starttime", "war_type")
            qDebug() << "\tbkParser::mailInbox" << mc << QJsonDocument(mc).toJson().data();
            continue;
        }
        //QString fightType = mc.value("fight_type").toString();
        bkReport* report = world->report(mc.value("fight_id").toString().toInt());
        if(report->isEmpty()) {
            //qDebug() << "\tbkParser::mailInbox" << report->id() << "isEmpty()";
            QString un;
            QJsonDocument result;
            // unserialize
            // aggressor, defender, fight_log, fight_stats
            un = mc.value("aggressor").toString().replace("O:13:\"FighterObject\":","a:");
            result = unserialize(un);
            if(result.object().value("type").toString() == "human") {
                //qDebug() << "aggressor:" << QJsonDocument(result).toJson().data();
                translatePlayer(result.object());
            }
            mc.insert("aggressor",QJsonValue(result.object()));

            un = mc.value("defender").toString().replace("O:13:\"FighterObject\":","a:");
            result = unserialize(un);
            if(result.object().value("type").toString() == "human") {
                //qDebug() << "defender:" << QJsonDocument(result).toJson().data();
                translatePlayer(result.object());
            }
            mc.insert("defender",QJsonValue(result.object()));

            un = mc.value("fight_log").toString();
            result = unserialize(un);
            //qDebug() << "--->fight_log:" << result.array();
            mc.insert("fight_log",QJsonValue(result.array()));

            un = mc.value("fight_stats").toString();
            result = unserialize(un);
            mc.insert("fight_stats",QJsonValue(result.object()));
            /*
             * aggressor:
             * replace("O:13:\"FighterObject\":","a:")
             * QJsonValue(string, "O:13:\"FighterObject\":53:{s:2:\"id\";i:28545;s:13:\"involvedUsers\";i:1;s:4:\"type\";s:5:\"human\";s:4:\"name\";s:11:\"DaimonMicha\";s:7:\"picture\";s:13:\"charMaleEvil3\";s:9:\"pictureID\";i:10;s:11:\"achievement\";s:0:\"\";s:5:\"level\";i:50;s:6:\"silver\";i:1317;s:12:\"silverToLoot\";i:1317;s:6:\"rubies\";i:251;s:5:\"karma\";i:-3924;s:10:\"experience\";i:12126;s:6:\"health\";i:2839;s:9:\"combatant\";s:9:\"aggressor\";s:8:\"hitZones\";a:5:{i:0;s:1:\"2\";i:1;s:1:\"7\";i:2;s:1:\"4\";i:3;s:1:\"3\";i:4;s:1:\"1\";}s:11:\"defendZones\";a:5:{i:0;s:1:\"1\";i:1;s:1:\"4\";i:2;s:1:\"4\";i:3;s:1:\"3\";i:4;s:1:\"2\";}s:8:\"strength\";i:283;s:9:\"dexterity\";i:251;s:9:\"endurance\";i:114;s:4:\"luck\";i:365;s:6:\"weapon\";i:371;s:6:\"shield\";i:333;s:9:\"damageMin\";i:197;s:9:\"damageMax\";i:239;s:6:\"armour\";d:187.19999999999999;s:24:\"armourFromMysticalShield\";i:0;s:10:\"bonusSpeed\";i:19;s:19:\"bonusCriticalDamage\";i:19;s:11:\"bonusDamage\";i:0;s:18:\"damageDuringCombat\";i:0;s:12:\"treasureItem\";i:0;s:17:\"premiumNormalizer\";i:1;s:11:\"clanPremium\";a:3:{s:4:\"wall\";b:0;s:5:\"dummy\";b:0;s:13:\"treasureChest\";b:0;}s:7:\"premium\";a:6:{s:7:\"account\";b:0;s:20:\"potionOfRegeneration\";b:0;s:6:\"guards\";b:0;s:5:\"dummy\";b:0;s:13:\"treasureChest\";b:0;s:13:\"brotherInArms\";b:0;}s:15:\"elementalDamage\";a:4:{s:4:\"fire\";i:26;s:3:\"ice\";i:20;s:5:\"shock\";i:25;s:6:\"poison\";i:39;}s:19:\"elementalResistance\";a:4:{s:4:\"fire\";d:14.300000000000001;s:3:\"ice\";d:13.199999999999999;s:5:\"shock\";d:13.199999999999999;s:6:\"poison\";d:36.299999999999997;}s:25:\"calculatedElementalDamage\";a:4:{s:4:\"fire\";i:23;s:3:\"ice\";i:18;s:5:\"shock\";i:25;s:6:\"poison\";i:39;}s:28:\"calculatedElementalDamageSum\";i:105;s:16:\"activeKarmaForce\";s:0:\"\";s:16:\"specialAbilities\";a:8:{s:19:\"SetitemDragonslayer\";i:2;s:14:\"SetitemGryphon\";i:3;s:25:\"ModifyMissionSilverReward\";i:8;s:19:\"ReduceTrainingCosts\";i:-3;s:10:\"ModifyLuck\";i:3;s:24:\"HorseTravelTimeReduction\";i:50;s:25:\"ModifyUserStatResistences\";i:10;s:16:\"ModifyDuelSilver\";N;}s:8:\"humanIDs\";a:0:{}s:6:\"gender\";s:1:\"m\";s:13:\"enemyDefeated\";i:8;s:17:\"display_damageMin\";i:307;s:17:\"display_damageMax\";i:349;s:13:\"baseDamageMin\";i:50;s:13:\"baseDamageMax\";i:50;s:13:\"itemDamageMin\";i:147;s:13:\"itemDamageMax\";i:189;s:16:\"baseItemStrength\";i:83;s:12:\"baseStrength\";i:200;s:14:\"display_armour\";i:234;}")
             *
             * defender:
             * replace("O:13:\"FighterObject\":","a:")
             * QJsonValue(string, "O:13:\"FighterObject\":53:{s:2:\"id\";i:1409;s:13:\"involvedUsers\";i:1;s:4:\"type\";s:5:\"human\";s:4:\"name\";s:7:\"asbesto\";s:7:\"picture\";s:13:\"charMaleEvil4\";s:9:\"pictureID\";i:12;s:11:\"achievement\";s:0:\"\";s:5:\"level\";i:46;s:6:\"silver\";i:0;s:12:\"silverToLoot\";i:0;s:6:\"rubies\";i:10;s:5:\"karma\";i:-342;s:10:\"experience\";i:10512;s:6:\"health\";i:1;s:9:\"combatant\";s:8:\"defender\";s:8:\"hitZones\";a:5:{i:0;s:1:\"5\";i:1;s:1:\"1\";i:2;s:1:\"8\";i:3;s:1:\"3\";i:4;s:1:\"5\";}s:11:\"defendZones\";a:5:{i:0;s:1:\"1\";i:1;s:1:\"2\";i:2;s:1:\"3\";i:3;s:1:\"1\";i:4;s:1:\"2\";}s:8:\"strength\";i:291;s:9:\"dexterity\";i:106;s:9:\"endurance\";i:103;s:4:\"luck\";i:137;s:6:\"weapon\";i:127;s:6:\"shield\";i:109;s:9:\"damageMin\";i:206;s:9:\"damageMax\";i:245;s:6:\"armour\";d:96.799999999999997;s:24:\"armourFromMysticalShield\";i:0;s:10:\"bonusSpeed\";i:23;s:19:\"bonusCriticalDamage\";i:28;s:11:\"bonusDamage\";i:0;s:18:\"damageDuringCombat\";i:0;s:12:\"treasureItem\";i:0;s:17:\"premiumNormalizer\";i:1;s:11:\"clanPremium\";a:3:{s:4:\"wall\";b:0;s:5:\"dummy\";b:0;s:13:\"treasureChest\";b:0;}s:7:\"premium\";a:6:{s:7:\"account\";b:0;s:20:\"potionOfRegeneration\";b:0;s:6:\"guards\";b:0;s:5:\"dummy\";b:0;s:13:\"treasureChest\";b:0;s:13:\"brotherInArms\";b:0;}s:15:\"elementalDamage\";a:4:{s:4:\"fire\";i:10;s:3:\"ice\";i:10;s:5:\"shock\";i:11;s:6:\"poison\";i:5;}s:19:\"elementalResistance\";a:4:{s:4:\"fire\";i:10;s:3:\"ice\";i:9;s:5:\"shock\";i:0;s:6:\"poison\";i:0;}s:25:\"calculatedElementalDamage\";a:4:{s:4:\"fire\";i:8;s:3:\"ice\";i:8;s:5:\"shock\";i:9;s:6:\"poison\";i:3;}s:28:\"calculatedElementalDamageSum\";i:28;s:16:\"activeKarmaForce\";s:0:\"\";s:16:\"specialAbilities\";a:5:{s:16:\"ModifyWorkSalary\";i:10;s:24:\"HorseTravelTimeReduction\";i:25;s:31:\"ChanceToHitBlockedStrikeAtJoust\";i:20;s:23:\"ChanceToBlockHitAtJoust\";i:5;s:16:\"ModifyDuelSilver\";N;}s:8:\"humanIDs\";a:0:{}s:6:\"gender\";s:1:\"m\";s:13:\"enemyDefeated\";i:7;s:17:\"display_damageMin\";i:242;s:17:\"display_damageMax\";i:281;s:13:\"baseDamageMin\";i:27;s:13:\"baseDamageMax\";i:27;s:13:\"itemDamageMin\";i:179;s:13:\"itemDamageMax\";i:218;s:16:\"baseItemStrength\";i:184;s:12:\"baseStrength\";i:107;s:14:\"display_armour\";i:121;}")
             *
             * fight_log:
             * QJsonValue(string, "a:26:{i:0;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:13:\"criticalPunch\";s:4:\"what\";a:0:{}}i:1;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:3:\"hit\";s:4:\"what\";a:4:{s:6:\"damage\";i:262;s:14:\"physicalDamage\";i:176;s:15:\"elementalDamage\";i:86;s:6:\"points\";i:300;}}i:2;a:3:{s:3:\"who\";s:10:\"human21978\";s:4:\"type\";s:9:\"missedHit\";s:4:\"what\";a:0:{}}i:3;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:13:\"criticalPunch\";s:4:\"what\";a:0:{}}i:4;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:3:\"hit\";s:4:\"what\";a:4:{s:6:\"damage\";i:217;s:14:\"physicalDamage\";i:131;s:15:\"elementalDamage\";i:86;s:6:\"points\";i:300;}}i:5;a:3:{s:3:\"who\";s:10:\"human21978\";s:4:\"type\";s:9:\"missedHit\";s:4:\"what\";a:0:{}}i:6;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:13:\"criticalPunch\";s:4:\"what\";a:0:{}}i:7;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:3:\"hit\";s:4:\"what\";a:4:{s:6:\"damage\";i:233;s:14:\"physicalDamage\";i:147;s:15:\"elementalDamage\";i:86;s:6:\"points\";i:300;}}i:8;a:3:{s:3:\"who\";s:10:\"human21978\";s:4:\"type\";s:9:\"missedHit\";s:4:\"what\";a:0:{}}i:9;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:13:\"criticalPunch\";s:4:\"what\";a:0:{}}i:10;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:3:\"hit\";s:4:\"what\";a:4:{s:6:\"damage\";i:224;s:14:\"physicalDamage\";i:138;s:15:\"elementalDamage\";i:86;s:6:\"points\";i:300;}}i:11;a:3:{s:3:\"who\";s:10:\"human21978\";s:4:\"type\";s:9:\"missedHit\";s:4:\"what\";a:0:{}}i:12;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:14:\"parriesTheDrub\";s:4:\"what\";a:0:{}}i:13;a:3:{s:3:\"who\";s:10:\"human21978\";s:4:\"type\";s:9:\"missedHit\";s:4:\"what\";a:0:{}}i:14;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:13:\"criticalPunch\";s:4:\"what\";a:0:{}}i:15;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:3:\"hit\";s:4:\"what\";a:4:{s:6:\"damage\";i:236;s:14:\"physicalDamage\";i:150;s:15:\"elementalDamage\";i:86;s:6:\"points\";i:300;}}i:16;a:3:{s:3:\"who\";s:10:\"human21978\";s:4:\"type\";s:9:\"missedHit\";s:4:\"what\";a:0:{}}i:17;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:13:\"criticalPunch\";s:4:\"what\";a:0:{}}i:18;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:3:\"hit\";s:4:\"what\";a:4:{s:6:\"damage\";i:225;s:14:\"physicalDamage\";i:139;s:15:\"elementalDamage\";i:86;s:6:\"points\";i:300;}}i:19;a:3:{s:3:\"who\";s:10:\"human21978\";s:4:\"type\";s:9:\"missedHit\";s:4:\"what\";a:0:{}}i:20;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:13:\"criticalPunch\";s:4:\"what\";a:0:{}}i:21;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:3:\"hit\";s:4:\"what\";a:4:{s:6:\"damage\";i:242;s:14:\"physicalDamage\";i:156;s:15:\"elementalDamage\";i:86;s:6:\"points\";i:300;}}i:22;a:3:{s:3:\"who\";s:10:\"human21978\";s:4:\"type\";s:9:\"missedHit\";s:4:\"what\";a:0:{}}i:23;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:13:\"criticalPunch\";s:4:\"what\";a:0:{}}i:24;a:3:{s:3:\"who\";s:10:\"human28545\";s:4:\"type\";s:3:\"hit\";s:4:\"what\";a:4:{s:6:\"damage\";i:204;s:14:\"physicalDamage\";i:118;s:15:\"elementalDamage\";i:86;s:6:\"points\";i:300;}}i:25;a:3:{s:3:\"who\";s:10:\"human21978\";s:4:\"type\";s:9:\"surrender\";s:4:\"what\";a:0:{}}}")
             *
             * fight_stats:
             * QJsonValue(string, "a:2:{s:9:\"aggressor\";a:9:{s:6:\"damage\";i:269;s:6:\"points\";i:569;s:6:\"health\";i:2839;s:11:\"surrendered\";b:0;s:10:\"fightItems\";a:0:{}s:9:\"oldHealth\";i:2839;s:6:\"silver\";d:0;s:10:\"experience\";i:-2;s:11:\"fightResult\";s:3:\"won\";}s:8:\"defender\";a:9:{s:6:\"damage\";i:0;s:6:\"points\";i:0;s:6:\"health\";i:1;s:11:\"surrendered\";b:1;s:10:\"fightItems\";a:0:{}s:9:\"oldHealth\";i:143;s:6:\"silver\";d:0;s:10:\"experience\";i:1;s:11:\"fightResult\";s:4:\"lost\";}}")
             *
             * phpProc->start("/usr/bin/php", QStringList() << "-r" << "echo json_encode(unserialize('"+data+"'));");
             */
            report->setData(mc);
        }
    }
}

QJsonDocument bkParser::unserialize(const QString& data)
{
    QProcess phpProc;
    QJsonDocument ret;
    phpProc.start("/usr/bin/php", QStringList() << "-r" << "echo json_encode(unserialize('"+data+"'));");
    if(!phpProc.waitForStarted()) {
        // error-handling!
        qDebug() << "error phpProc.waitForStarted()!";
        return ret;
    }

    if(!phpProc.waitForFinished()) {
        // error-handling!
        qDebug() << "error phpProc.waitForFinished()!";
        return ret;
    }
    ret = QJsonDocument::fromJson(phpProc.readAllStandardOutput());
    //qDebug() << ret.toJson().data();
    return ret;
}

/*
 * {"combatant":"aggressor",
 * "strength":283,"dexterity":251,"endurance":114,"luck":365,"weapon":371,"shield":333,
 * "damageMin":197,"damageMax":239,"armour":187.2,"armourFromMysticalShield":0,
 * "bonusSpeed":19,"bonusCriticalDamage":19,"bonusDamage":0,
 * "damageDuringCombat":0,"treasureItem":0,"premiumNormalizer":1,
 * "clanPremium":{"wall":false,"dummy":false,"treasureChest":false},
 * "premium":{"account":false,"potionOfRegeneration":false,"guards":false,"dummy":false,"treasureChest":false,"brotherInArms":false},
 * "elementalDamage":{"fire":26,"ice":20,"shock":25,"poison":39},
 * "elementalResistance":{"fire":14.3,"ice":13.2,"shock":13.2,"poison":36.3},
 * "calculatedElementalDamage":{"fire":0,"ice":18,"shock":22,"poison":27},
 * "calculatedElementalDamageSum":67,
 * "activeKarmaForce":"",
 * "specialAbilities":{"SetitemDragonslayer":2,"SetitemGryphon":3,"ModifyMissionSilverReward":8,"ReduceTrainingCosts":-3,"ModifyLuck":3,"HorseTravelTimeReduction":50,"ModifyUserStatResistences":10},
 * "enemyDefeated":8,"display_damageMin":307,"display_damageMax":349,
 * "baseDamageMin":50,"baseDamageMax":50,"itemDamageMin":147,"itemDamageMax":189,
 * "baseItemStrength":83,"baseStrength":200,"display_armour":234}
 *
 * {"id":18870,"involvedUsers":1,"type":"human","name":"Das Fantom","picture":"charMaleGood1","pictureID":3,"achievement":"","level":66,"silver":34939,"silverToLoot":34939,"rubies":45,"karma":234,"experience":21426,"health":5445,"combatant":"aggressor","hitZones":["6","9","3"],"defendZones":["3","1","5"],"strength":385,"dexterity":295,"endurance":237,"luck":230,"weapon":293,"shield":306,"damageMin":282,"damageMax":299,"armour":238.4,"armourFromMysticalShield":0,"bonusSpeed":18,"bonusCriticalDamage":10,"bonusDamage":0,"damageDuringCombat":0,"treasureItem":0,"premiumNormalizer":1,"clanPremium":{"wall":false,"dummy":false,"treasureChest":false},"premium":{"account":false,"potionOfRegeneration":false,"guards":false,"dummy":false,"treasureChest":false,"brotherInArms":false},"elementalDamage":{"fire":0,"ice":23,"shock":23,"poison":28},"elementalResistance":{"fire":0,"ice":36.3,"shock":38.5,"poison":42.9},"calculatedElementalDamage":{"fire":0,"ice":18,"shock":21,"poison":17},"calculatedElementalDamageSum":56,"activeKarmaForce":"","specialAbilities":{"SetitemDragonslayer":4,"ChanceForBonusGMItem":5,"ChanceForItemAtMission":1,"HorseTravelTimeReduction":25,"ModifyUserStatEndurance":10,"ModifyUserStatResistences":10},"humanIDs":[],"gender":"m","enemyDefeated":9,"display_damageMin":356,"display_damageMax":373,"baseDamageMin":55,"baseDamageMax":55,"itemDamageMin":227,"itemDamageMax":244,"baseItemStrength":165,"baseStrength":220,"display_armour":298}
 */
void bkParser::translatePlayer(const QJsonObject& player)
{
    bkPlayer* p = p_account->world()->player(player.value("id").toInt());
    QJsonObject tp;
    QJsonArray zones;
    tp.insert("knight_id",player.value("id"));
    tp.insert("knight_name",player.value("name"));
    tp.insert("knight_level",player.value("level"));
    tp.insert("silver",player.value("silver"));
    tp.insert("rubies",player.value("rubies"));
    tp.insert("karma",player.value("karma"));
    tp.insert("experience",player.value("experience"));
    tp.insert("gender",player.value("gender"));

    zones = player.value("hitZones").toArray();
    if(zones.count() == 5) tp.insert("hitZones", zones);
    if(zones.count() == 3) tp.insert("turnierHitZones", zones);
    zones = player.value("defendZones").toArray();
    if(zones.count() == 5) tp.insert("defendZones", zones);
    if(zones.count() == 3) tp.insert("turnierDefendZones", zones);

    tp.insert("strength",player.value("strength"));

    p->setData(tp);
}


void bkParser::ajaxGetInventory(const QByteArray& data)
{
    QJsonDocument json = QJsonDocument::fromJson(data);
    if(!json.object().value("result").toBool()) return;

    bkWorld* world = p_account->world();
    QJsonArray items = json.object().value("items").toArray();
    foreach(QJsonValue iv, items) {
        QJsonObject io = iv.toObject();
        bkItem* item = world->item(io.value("item_id").toString().toInt());
        item->setData(io);
        //QJsonDocument debug(iv.toObject()); qDebug() << "\t" << debug.toJson().data();
    }

    //qDebug() << "\t" << json.toJson().data();
}

// /ajax/ajax/buyItem
void bkParser::ajaxBuyItem(const QByteArray& data)
{
    QJsonDocument json = QJsonDocument::fromJson(data);
    if(!json.object().value("result").toBool()) return;

    QJsonObject d = json.object().value("data").toObject();
    QJsonObject i = d.value("item").toObject();
    bkWorld* world = p_account->world();
    bkItem* item = world->item(i.value("item_id").toInt());
    item->setData(i);

    qDebug() << "\tbkParser::ajaxBuyItem" << item;
}

// /ajax/duel/proposals
void bkParser::ajaxProposal(const QByteArray& data)
{
/*
    QJsonDocument json = QJsonDocument::fromJson(data);
    if(!json.object().value("result").toBool()) return;

    QJsonObject po = json.object().value("data").toArray().at(0).toObject();
    bkWorld* world = p_account->world();
    bkPlayer* p = world->player(po.value("knight_id").toString().toInt());
    p->setData(po);
    //clanManager
    if(!po.value("clan_id").isNull()) {
        bkClan* clan = world->clan(po.value("clan_id").toInt());
        clan->setData(po);
    }

    QVariant num((int)p->id());
    p_account->setStatus("currentProposal", num);
*/
    //qDebug() << "\nbkParser::ajaxProposal(proposal)\n" << p;
}

/*
 * /tavern/battle
 */
void bkParser::tavernBattleHtml(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement main = doc.findFirst("#mainContent");
    if(main.isNull()) return;
    if(!main.classes().contains("battle")) return;
    QVariant progressbarEndTime = page->mainFrame()->evaluateJavaScript("if(typeof progressbarEndTime !== 'undefined') progressbarEndTime;");
    QVariant progressbarDuration = page->mainFrame()->evaluateJavaScript("if(typeof progressbarDuration !== 'undefined') progressbarDuration;");
    p_account->setStatus("timerOS", progressbarEndTime);
    p_account->setStatus("timerOSDuration", progressbarDuration);
    QWebElement battlerounds = main.findFirst("div.battlerounds");
    if(battlerounds.isNull()) return;
    QString round = battlerounds.toPlainText().trimmed().split(" ", QString::SkipEmptyParts).at(1);
    QVariant modus = round.toInt() + 2;
    p_account->setStatus("modusOS", modus);
    qDebug() << "bkParser::tavernBattleHtml" << progressbarEndTime.toInt() << progressbarDuration.toInt() << modus;
}

/*
 * /tavern/prepare
 */
void bkParser::tavernPrepareHtml(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement main = doc.findFirst("#mainContent");
    if(main.isNull()) return;
    if(!main.classes().contains("prepare")) return;
    QVariant progressbarEndTime = page->mainFrame()->evaluateJavaScript("if(typeof progressbarEndTime !== 'undefined') progressbarEndTime;");
    QVariant progressbarDuration = page->mainFrame()->evaluateJavaScript("if(typeof progressbarDuration !== 'undefined') progressbarDuration;");
    p_account->setStatus("timerOS", progressbarEndTime);
    p_account->setStatus("timerOSDuration", progressbarDuration);
    QVariant modus = 2;// eingezogen
    p_account->setStatus("modusOS", modus);
    qDebug() << "bkParser::tavernPrepareHtml" << progressbarEndTime.toInt() << progressbarDuration.toInt();
}
