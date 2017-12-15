#include "bkgmmodule.h"
#include "bkaccount.h"

#include <QNetworkReply>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>




bkGMModule::bkGMModule(QJsonObject& defaults, bkAccount* account, QObject *parent) :
    bkModule("GM", defaults, account, parent)
{
    addPath("/groupmission/");
    addPath("/groupmission/map");
    addPath("/groupmission/group/");

    addStateName(waiting, tr("waiting"));
    addStateName(no_points, tr("no_points"));
    addStateName(no_group, tr("no_group"));
    addStateName(group_found, tr("group_found"));
    addStateName(wearing, tr("wearing"));
    addStateName(join_group, tr("join_group"));
    addStateName(in_group, tr("in_group"));

    connect(&mTimer, SIGNAL(timeout()), this, SLOT(adjustPoints()));
    QTimer::singleShot(100, this, SLOT(adjustTimer()));
}

void bkGMModule::adjustTimer()
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    setState("timerProgress", (int)now.addSecs(3600).toTime_t());
    mTimer.start(3600 * 1000); // 1h
    qDebug() << QString("bk" + name() + "Module::adjustTimer(%1)").arg(state("timerProgress").toInt()).toLocal8Bit().data();
}

void bkGMModule::adjustPoints()
{
    int ist = state("points").toInt();
    if(ist < 120) {
        ist += 5;
        setState("points", ist);
        // GMUpdate aufrufen
        QWebPage* page = p_account->currentPage();
        page->mainFrame()->evaluateJavaScript("if(typeof GMUpdate === 'function') GMUpdate();");
        qDebug() << QString("bk" + name() + "Module::adjustPoints(%1)").arg(ist).toLocal8Bit().data();
    }
}

void bkGMModule::replyFinished(QNetworkReply* reply)
{
    QString dpath = reply->url().path().toLower();
    if(!hasPath(dpath)) return;

    qDebug() << QString("bk" + name() + "Module::replyFinished(POST)").toLocal8Bit().data() << reply->property("postData").toString();
}

void bkGMModule::getGroups(QWebElement groupTable)
{
    QWebElement tbody = groupTable.findFirst("tbody");
    QWebElementCollection groups = tbody.findAll("tr");
    foreach(QWebElement row, groups) {
        if(row.hasClass("noBreakRow")) {
            setState("state", no_group); // no_group_found
            qDebug() << "bkGMModule::getGroups(group_found)" << "...no groups found!";
            return;
        }
        QWebElement tester = row.findFirst("td.highscore02");
        QString levels = tester.toPlainText().trimmed();
        setState("state", group_found); // group_found
        qDebug() << "bkGMModule::getGroups(group_found)" << levels;

        // <tbody><tr><td class="highscore01"><a href="../groupmission/groupProfile/?groupID=52298">4</a></td>
        // <td class="highscore02">36 - 61</td>
        // <td class="highscore03 playerName"><a href="./common/profile/21616/Tavern/GroupMission">Vizegraf lordlulu</a> <a href="./common/orderprofile/5/Tavern/GroupMission">[GdM]</a></td>
        // <td class="highscore04">1</td>
        // <td class="highscore05">4</td>
        // <td class="highscore06">mittel</td>
        // <td class="highscore07">Nein</td>
        // <td scope="col" class="highscore08"><a class="thIcon checkIcon mediumToolTip"></a></td></tr></tbody>

        // QWebElement button = row.findFirst("a.checkIcon");
        // button.evaluateJavaScript("click()");
    }
}

void bkGMModule::groupInfo(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();
    //Group auslesen...
    QWebElement groupForm = doc.findFirst("form#formFound");
    if(groupForm.isNull()) return;
    QWebElementCollection divs = groupForm.findAll("div.clearfix");
    /*
     * <div class="formLine clearfix">
                <label>Automatischer Missionsstart</label>
                <div class="formField">
                    schwer						<div class="subLabel">* Voraussichtlicher Schwierigkeitsgrad. Falls nicht verfügbar wird der nächst leichtere gewählt.</div>
                </div>
            <!-- end .formLine-->
            </div>
     */
    if(divs.count() > 6) {
        QString start = divs.at(5).firstChild().nextSibling().toPlainText();
        start = start.left(start.indexOf("\n"));
        qDebug() << "vvv - inGroup: "//div.clearfix (count)" << divs.count()
                 << divs.at(4).firstChild().nextSibling().toPlainText()//aktuelle Anzahl Mitglieder
                 << "von"
                 << divs.at(3).firstChild().nextSibling().toPlainText()//maximale Anzahl Mitglieder
                 << start//Missionsstart
                 ;
    }
}

void bkGMModule::parse(QWebPage* page, QVariant titleTimer, QVariant, QVariant)
{
    QString dpath = page->mainFrame()->url().path().toLower();
    if(!hasPath(dpath)) return;

    int ist = state("points").toInt();
    int soll = ist;
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement groupTable = doc.findFirst("#highscoreClanTable");
    if(!groupTable.isNull()) {
        getGroups(groupTable);
        soll = 120;
    }

    QVariant inGroup = page->mainFrame()->evaluateJavaScript("if(typeof inGroup !== 'undefined') inGroup;");
    if(inGroup.isValid() && inGroup.toBool()) {
        setState("state", in_group);
        soll = 120;
        groupInfo(page);
        if(!active()) {
            setState("strength", 255);
            emit jobDone();
        }
    }

    QWebElement tester = doc.findFirst("div.innerContent");
    if(!tester.isNull()) {
        QWebElement p = tester.findFirst("em");
        if(!p.isNull()) {
            setState("state", no_points);
            soll = p.toPlainText().trimmed().toInt();
        }
    }

    if(titleTimer.isValid() && titleTimer.toInt() > 0) {
        // we have an cooldown
        soll = ist;
        if(state("state").toInt() == in_group) {
            setState("state", no_points); // no_group_found
            stopActionTimer();
            if(active()) emit jobDone();
        } else {
            setState("state", waiting);
        }
        setState("strength", 28);
    }

    if(ist != soll) {
        setState("points", soll);
        adjustTimer();
        if(active()) emit jobDone();
    }

    //qDebug() << QString("bk" + name() + "Module::parse (" + dpath + ")").toLocal8Bit().data() << this;
}

void bkGMModule::logic(QWebPage*)
{
    int strength = state("strength").toInt();
    switch(state("state").toInt()) {
        case no_points:
            if(state("points").toInt() > 110) strength += 2;
            break;
        case in_group:
            strength += 4;
            break;
        case wearing:
            setState("state", join_group);
            break;
        case group_found:
            emit hasJobFor("Account", "wear", wear_forGM);
            setState("state", wearing);
            break;
        default:
            break;
    }
    setState("strength", strength);
}

void bkGMModule::action(QWebPage* page)
{
    QString dpath = page->mainFrame()->url().path().toLower();
    switch(state("state").toInt()) {
        case no_points:
            startActionTimer(10*60*1000); // max 10min
            break;
        case join_group:
            break;
        case in_group:
            startActionTimer(25*60*1000); // max 25min
            break;
        default:
            break;
    }
    setState("strength", 32);

    //if(!hasPath(dpath)) return;
    //qDebug() << QString("bk" + name() + "Module::action(%1, %2)").arg(stateName(state("state").toInt())).arg(state("wait").toInt()).toLocal8Bit().data() << dpath;
}

void bkGMModule::clickTavern()
{
    QWebPage* page = p_account->currentPage();
    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement knob = doc.findFirst("#navTavern");
    if(!knob.isNull()) knob.evaluateJavaScript("this.click();");
}

void bkGMModule::actionTimer()
{
    setState("wait", 0);

    switch(state("state").toInt()) {
        case no_points:
        case in_group:
            clickTavern();
            break;
        default:
            break;
    }

    qDebug() << QString("bk" + name() + "Module::actionTimer(%1)").arg(stateName(state("state").toInt())).toLocal8Bit().data();
}

void bkGMModule::view(QWebPage* page)
{
    bkModule::view(page);
    //if(!state("enable").toBool()) return;
    QWebElement moduleElement = getModule(page->mainFrame()->documentElement());

    QWebElement text = moduleElement.findFirst("#kmGMPoints");
    if(!text.isNull()) return;
    QVariant points = state("points").toVariant();
    if(!points.isValid()) return;
    QString content;
    text = appendLine(moduleElement);

    //if(!state("enable").toBool()) return;

    content.append(QString(" (<span>%1</span>)").arg(points.toInt()));
    text.appendInside(content);
    text.setAttribute("id", "kmGMPoints");
    text.setAttribute("align", "right");

    QString script = "var GMUpdate = function() {\n"
                     "  var text = parseInt(account.state('GM','points'));\n"
                     "  document.id('kmGMPoints').getElement('span').set('text', text);\n"
                     "}\n"
                     "GMUpdate();\n";
    page->mainFrame()->evaluateJavaScript(script);

    //qDebug() << QString("bk"+name()+"Module::view").toLocal8Bit().data() << this;
}
