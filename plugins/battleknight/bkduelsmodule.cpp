#include "bkduelsmodule.h"
#include "bkaccount.h"
#include "bkplayer.h"

#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QDebug>




bkDuelsModule::bkDuelsModule(QJsonObject& defaults, bkAccount* account, QObject *parent) :
    bkModule("Duels", defaults, account, parent)
{
    addPath("/duel/");
    addPath("/duel/compare/");
    addPath("/duel/duel/");

    addStateName(vote, tr("first check"));
    addStateName(go_tournament, tr("go arena"));
    addStateName(next_proposal, tr("next proposal"));
    addStateName(to_compare, tr("go compare"));
    addStateName(compare, tr("second check"));
    addStateName(attack, tr("go fight"));
    addStateName(result, tr("read result"));
    addStateName(cooldown, tr("cooldown"));
}

void bkDuelsModule::replyFinished(QNetworkReply* reply)
{
    QString dpath = reply->url().path().toLower();

    if(dpath == "/ajax/duel/proposals/") {
        QByteArray data = reply->property("getData").toByteArray();
        QJsonDocument json = QJsonDocument::fromJson(data);
        if(!json.object().value("result").toBool()) return;

        QJsonObject po = json.object().value("data").toArray().at(0).toObject();
        int pid = po.value("knight_id").toString().toInt();
        bkPlayer* p = p_account->world()->player(pid);
        p->setData(po);
        //clanManager
        if(!po.value("clan_id").isNull()) {
            bkClan* clan = p_account->world()->clan(po.value("clan_id").toInt());
            clan->setData(po);
        }

        setState("currentProposal", p->id());
        if(active()) {
            setState("state", vote);
            logic(p_account->currentPage());
            action(p_account->currentPage());
        }

        //qDebug() << QString("bk" + name() + "Module::replyFinished (" + dpath + ")").toLocal8Bit().data() << state("state").toInt() << p;// << this;
    }

}

void bkDuelsModule::duelHtml(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement main = doc.findFirst("#mainContent");
    if(main.isNull()) return;
    QWebElement proposal = doc.findFirst("#proposals").firstChild();
    if(proposal.isNull()) return;
    QWebElementCollection links = proposal.findAll("a");
    if(links.count() == 0) return;
    QWebElement tester = links.at(0);
    QUrl url(tester.attribute("href"));
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
    setState("currentProposal", (int)p->id());
    //QVariant num((int)p->id());
    //p_account->setStatus("currentProposal", num);

    //qDebug() << "bkDuelsModule::duelHtml(proposal):" << p;
             //<< QJsonDocument(po).toJson().data();
}

void bkDuelsModule::storePlayer(int pid, QWebElement playerRow)
{
    QJsonObject data;
    bkPlayer* player = p_account->player(pid);
    QWebElementCollection rows = playerRow.findAll("td");
    if(rows.count() < 8) return;

    data.insert("knight_id", pid);
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
    qDebug() << "bkDuelsModule::storePlayer()" << player;
}

void bkDuelsModule::duelCompareHtml(QWebPage* page)
{
    QString path = page->mainFrame()->url().path();
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement tester = doc.findFirst("#profileAttrib");
    if(tester.isNull()) return;

    storePlayer(p_account->player()->id(), tester);

    QUrlQuery query(page->mainFrame()->url().query());
    // enemyID
    int nr = query.queryItemValue("enemyID").toInt();
    tester = doc.findFirst("#challengerAttrib");

    storePlayer(nr, tester);
/*
    player = p_account->player(nr);
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
*/
}

void bkDuelsModule::duelDuelHtml(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement main = doc.findFirst("#mainContent");
    if(main.isNull()) return;
    if(!main.classes().contains("duelFightResult")) return;

    qDebug() << "bkDuelsModule::duelDuelHtml";
/*
    QVariant titleTimer = page->mainFrame()->evaluateJavaScript("if(typeof l_titleTimerEndTime !== 'undefined') l_titleTimerEndTime;");
    int wait = qrand() % ((titleTimer.toInt()*1000)/6) + 123;
    qDebug() << "\n" << "---> /duel/duel (titleTimer)" << titleTimer.toInt() << wait;
    QTimer::singleShot(wait, this, SLOT(callReports()));
*/
}

void bkDuelsModule::parse(QWebPage* page, QVariant titleTimer, QVariant, QVariant)
{
    stopActionTimer();
    QString dpath = page->mainFrame()->url().path().toLower();

    if(!hasPath(dpath)) {
        if(state("state").toInt() == result) {
            setState("state", go_tournament);
            if(active()) emit jobDone();
            return;
        }
    }

    if(titleTimer.isValid() && !titleTimer.isNull()) {
        QWebElement doc = page->mainFrame()->documentElement();
        QWebElement main = doc.findFirst("#mainContent");
        qDebug() << "bkDuelsModule::parse(mainContent + titleTimer)" << main.classes();

        setState("state", cooldown);
        if(dpath == "/duel/duel/") {
            duelDuelHtml(page);

            setState("state", result);
            setState("wait", (titleTimer.toInt()*1000)/6);

            emit jobDone();
        }
    } else {

        if(dpath == "/duel/") {
            duelHtml(page); // hier aktivieren
            setState("state", vote);
        } else if(dpath == "/duel/compare/") {
            duelCompareHtml(page);
            setState("state", compare);
        } else if(dpath == "/duel/duel/") {
            //if(active()) emit jobDone();
        }

    }

    if(!hasPath(dpath)) {
        setState("state", go_tournament);
        return;
    }

    //if(!hasPath(dpath)) return;
    //qDebug() << QString("bk" + name() + "Module::parse (" + dpath + ")").toLocal8Bit().data() << this;
}

void bkDuelsModule::logic(QWebPage* page)
{
    QString dpath = page->mainFrame()->url().path().toLower();

    int strength = state("strength").toInt();
    switch(state("state").toInt()) {
        case vote:
            // lazy check, eg. has Clan?
            if(state("currentProposal").toInt() > 0) {
                bkPlayer* knight = p_account->player(state("currentProposal").toInt());
                bkPlayer* player = p_account->player();
                if(player->canFight(knight)) {
                    setState("state", to_compare);
                } else {
                    setState("state", next_proposal);
                }
            }
            break;
        case compare:
            // do a better check, we know the enemy here...
            setState("state", attack);
            break;
        default:
            break;
    }
    ++strength;
    setState("strength", strength);
}

void bkDuelsModule::action(QWebPage* page)
{
    QString dpath = page->mainFrame()->url().path().toLower();

    int strength = state("strength").toInt();
    switch(state("state").toInt()) {
        case vote:
            break;
        case next_proposal:
            startActionTimer(((6)*1000)); // max ½min
            break;
        case to_compare:
            startActionTimer(((30)*1000)); // max ½min
            break;
        case attack:
            //                   min  sec
            startActionTimer(((1*60)*1000)); // max 1min
            break;
        case go_tournament:
        case result:
            if(state("wait").toInt() > 0) {
                startActionTimer(state("wait").toInt()); // max (titleTimer/6)
            } else {
                //                   min  sec
                startActionTimer(((6*60)*1000)); // max 6min
            }
            break;
        default:
            break;
    }
    setState("strength", strength);

    if(!hasPath(dpath)) return;
}

void bkDuelsModule::clickWeaker()
{
    QWebPage* page = p_account->currentPage();
    page->mainFrame()->evaluateJavaScript("requestProposals('weaker');");
}

void bkDuelsModule::clickStronger()
{
    QWebPage* page = p_account->currentPage();
    page->mainFrame()->evaluateJavaScript("requestProposals('stronger');");
}

void bkDuelsModule::clickCompare()
{
    QWebPage* page = p_account->currentPage();
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement knob = doc.findFirst("#devBtnNext");
    if(!knob.isNull()) knob.evaluateJavaScript("this.click();");
}

void bkDuelsModule::clickAttack()
{
    QWebPage* page = p_account->currentPage();
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement knob = doc.findFirst("#devAttackBtn");
    if(!knob.isNull()) knob.evaluateJavaScript("this.click();");
}

void bkDuelsModule::clickTournament()
{
    QWebPage* page = p_account->currentPage();
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement knob = doc.findFirst("#navTournamentDuel");
    if(!knob.isNull()) knob.evaluateJavaScript("this.click();");
}

void bkDuelsModule::actionTimer()
{
    setState("wait", 0);
    switch(state("state").toInt()) {
        case next_proposal:
            clickWeaker();
            break;
        case to_compare:
            clickCompare();
            break;
        case attack:
            clickAttack();
            break;
        case result:
            clickTournament();
            if(active()) emit jobDone();
            break;
        case go_tournament:
            clickTournament();
            break;
        default:
            break;
    }
    qDebug() << QString("bk" + name() + "Module::actionTimer(%1)").arg(stateName(state("state").toInt())).toLocal8Bit().data();
}

void bkDuelsModule::view(QWebPage* page)
{
    QWebElement moduleElement = getModule(page->mainFrame()->documentElement());
    QWebElement checkbox = moduleElement.findFirst("#enable" + name());
    if(checkbox.isNull()) return;
    if(state("enable").toBool()) checkbox.setAttribute("checked","checked");
    checkbox.evaluateJavaScript("this.addEventListener('click', function(){ account.toggle(this.id, this.checked); });");
    if(!state("enable").toBool()) return;

    //qDebug() << QString("bk"+name()+"Module::view").toLocal8Bit().data() << this;
}
