#include "bkmodulemanager.h"

// sichtbare module
#include "bkaccountmodule.h"
#include "bkduelsmodule.h"
#include "bkgmmodule.h"
#include "bkmissionsmodule.h"
#include "bkosmodule.h"
#include "bktournamentmodule.h"
#include "bktreasurymodule.h"
#include "bkworkmodule.h"

// unsichtbare module
#include "bktravelmodule.h"
#include "bkreportsmodule.h"
#include "bkhighscoremodule.h"

#include <QNetworkReply>
#include <QWebPage>
#include <QWebElement>
#include <QJsonDocument>

#include <QDebug>







bkModuleManager::bkModuleManager(QJsonObject& defaults, QObject *parent) :
    QObject(parent)
{
    p_account = qobject_cast<bkAccount*>(parent);
    Q_ASSERT(p_account);
    m_settings = QJsonObject(defaults);

    m_reloadPaths.append("/duel/");
    m_reloadPaths.append("/duel/duel/");
    m_reloadPaths.append("/world");
    m_reloadPaths.append("/world/location");
    m_reloadPaths.append("/world/travel");
    m_reloadPaths.append("/groupmission/group/");
    m_reloadPaths.append("/market/work");

    // sichtbare module
    p_accountModul = addModule("Account");
    addModule("Duels");
    addModule("GM");
    addModule("Missions");
    addModule("OS");
    addModule("Tournament");
    addModule("Treasury");
    addModule("Work");

    // unsichtbare module
    addModule("Travel");
    addModule("Reports");
    addModule("Highscore");

    //qDebug() << "bkModuleManager" << "created." << defaults.keys();
}

bkModuleManager::~bkModuleManager()
{
    while(m_modules.count() > 0) {
        delete m_modules.takeFirst();
    }
    qDebug() << "bkModuleManager" << "destroyed.";
}

bkModule* bkModuleManager::addModule(const QString& module)
{
    if(!m_settings.contains(module)) return NULL;
    QJsonObject config = m_settings.value(module).toObject();

    QPointer<bkModule> modul;
    if(module == "Account") modul = new bkAccountModule(config, p_account, this);
    else if(module == "Duels") modul = new bkDuelsModule(config, p_account, this);
    else if(module == "GM") modul = new bkGMModule(config, p_account, this);
    else if(module == "Missions") modul = new bkMissionsModule(config, p_account, this);
    else if(module == "OS") modul = new bkOSModule(config, p_account, this);
    else if(module == "Tournament") modul = new bkTournamentModule(config, p_account, this);
    else if(module == "Treasury") modul = new bkTreasuryModule(config, p_account, this);
    else if(module == "Work") modul = new bkWorkModule(config, p_account, this);

    else if(module == "Travel") modul = new bkTravelModule(config, p_account, this);
    else if(module == "Reports") modul = new bkReportsModule(config, p_account, this);
    else if(module == "Highscore") modul = new bkHighscoreModule(config, p_account, this);

    if(!modul.isNull()) {
        m_modules.append(modul);
        connect(modul, SIGNAL(jobDone()), this, SLOT(moduleIsReady()));
        connect(modul, SIGNAL(hasJobFor(QString,QString,int)), this, SLOT(moduleHasJobFor(QString,QString,int)));
    }
    return modul;
}

void bkModuleManager::restoreState(QJsonObject& state)
{
    foreach(QPointer<bkModule> modul, m_modules) {
        if(state.contains(modul->name())) {
            QJsonObject config = state.value(modul->name()).toObject();
            modul->restoreState(config);
            if(modul->active()) {
                if(modul != p_activeModul) modul->deactivate();
            }
            //qDebug() << "--> bkModuleManager::restoreState()" << modul;
        }
    }
}

QJsonObject bkModuleManager::saveState()
{
    QJsonObject state;
    foreach(QPointer<bkModule> modul, m_modules) {
        state.insert(modul->name(), modul->saveState());
    }
    //qDebug() << "--> bkModuleManager::saveState" << state.keys();// << QJsonDocument(state).toJson().data();
    return state;
}

void bkModuleManager::replyFinished(QNetworkReply* reply)
{
    foreach(QPointer<bkModule> modul, m_modules) {
        modul->replyFinished(reply);
    }
}

void bkModuleManager::setState(const QString& key, QJsonValue value)
{
    m_state.insert(key, value);
}

QJsonValue bkModuleManager::state(const QString& key)
{
    QJsonValue ret;

    if(m_state.contains(key)) {
        ret = m_state.value(key);
        //qDebug() << "bkModuleManager::state" << key << ret;
        return ret;
    }

    return ret;
}

void bkModuleManager::loadFinished(QWebPage* page)
{
    QString dpath = page->mainFrame()->url().path().toLower();

    // cooldownTimer
    QVariant titleTimer = page->mainFrame()->evaluateJavaScript("if(typeof l_titleTimerEndTime !== 'undefined') l_titleTimerEndTime;");
    // progressbarEndTime
    QVariant progressbarEndTime = page->mainFrame()->evaluateJavaScript("if(typeof progressbarEndTime !== 'undefined') progressbarEndTime;");
    // progressbarDuration
    QVariant progressbarDuration = page->mainFrame()->evaluateJavaScript("if(typeof progressbarDuration !== 'undefined') progressbarDuration;");

    // check cooldown-modus
    if(!titleTimer.isNull() && m_reloadPaths.contains(dpath)) {

        QWebElement tester = page->mainFrame()->findFirstElement("#mainContent");
        if(!tester.isNull()) {
            int modus(cd_none);
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
            } else if(tester.classes().contains("fightResult")) { // "/duel/duel/?enemyID=53287" ("fightResult", "duelFightResult")
                if(tester.classes().contains("duelFightResult")) {
                    modus = cd_duel;
                    setState("moduleCooldown", "Duels");
                } else {
                    modus = cd_fight;
                    setState("moduleCooldown", "Mission");
                }
            }
            if(modus != cd_none) {
                setState("modusCooldown", modus);
                if(progressbarEndTime.isValid()) setState("timerCooldown", progressbarEndTime.toInt() + 2);
                if(progressbarDuration.isValid()) setState("timerCooldownDuration", progressbarDuration.toInt()/* + 1*/);
            }
        }

    } else { // check cooldown-modus
        setState("modusCooldown", cd_none);
    }
/*
<div id="nav">
                                <ul>

    <li class="toggler ">
        <a id="navCharacter" href="https://s12-de.battleknight.gameforge.com:443/user/">Charakter<span id="devCountNewLootitems">2</span></a>
            </li>

    <li class="toggler ">
        <a id="navJourney" href="https://s12-de.battleknight.gameforge.com:443/world">Welt</a>
            </li>
    <li class="toggler ">
        <a id="navMessages" href="https://s12-de.battleknight.gameforge.com:443/mail/index/menu">Nachrichten<span id="devCountNewMails">1</span></a>
            </li>
*/
    // check for news
    QWebElement nav = page->mainFrame()->findFirstElement("div#nav");
    if(!nav.isNull()) {
        QWebElementCollection toggler = nav.findAll("li.toggler");
        foreach(QWebElement knob, toggler) {
            QWebElement link = knob.findFirst("a");
            if(link.isNull()) continue;
            QWebElement news = link.findFirst("span");
            if(news.isNull()) continue;
            qDebug() << QString("bkModuleManager::parse(news): %1").arg(news.attribute("id")).toLocal8Bit().data();
        }
    }

    // Parse-Loop
    foreach(QPointer<bkModule> modul, m_modules) {
        modul->parse(page, titleTimer, progressbarEndTime, progressbarDuration);
        if(!modul->enabled() || !modul->hasPath(dpath)) continue;
        qDebug() << QString("bk" + modul->name() + "Module::parse (" + dpath + ")").toLocal8Bit().data() << modul;
    }

    logic(page);

    qDebug() << QString("bkModuleManager::parse()").toLocal8Bit().data() << QJsonDocument(m_state).toJson().data();

    if(p_activeModul.isNull()) {
        foreach(QPointer<bkModule> modul, m_modules) {
            if(p_activeModul.isNull()) {
                if(modul->enabled()) p_activeModul = modul;
            } else {
                if(modul->enabled() && p_activeModul->strength() < modul->strength()) p_activeModul = modul;
            }
        }
        if(p_activeModul.isNull())  return;
        p_activeModul->activate();
    }

    view(page);

    if(!p_accountModul || !p_accountModul->enabled()) return;
    // made it happen
    if(!p_activeModul || !p_activeModul->enabled()) return;
    p_activeModul->action(page);
    qDebug() << QString("bk" + p_activeModul->name() + "Module::action(%1, %2)")
                .arg(p_activeModul->stateName(p_activeModul->state("state").toInt()))
                .arg(p_activeModul->state("wait").toInt()).toLocal8Bit().data() << dpath;
}

void bkModuleManager::logic(QWebPage* page)
{
    if(!p_accountModul || !p_accountModul->enabled()) return;

    QString dpath = page->mainFrame()->url().path().toLower();
    // Logic-Loop
    foreach(QPointer<bkModule> modul, m_modules) {
        if(modul->enabled()) {
            modul->logic(page);
            qDebug() << QString("bk" + modul->name() + "Module::logic(%1, %2)")
                        .arg(modul->stateName(modul->state("state").toInt()))
                        .arg(modul->strength())
                        .toLocal8Bit().data() << dpath;// << this;
        }
    }
}

void bkModuleManager::view(QWebPage* page)
{
    QString dpath = page->mainFrame()->url().path().toLower();
    // View-Loop
    foreach(QPointer<bkModule> modul, m_modules) {
        modul->view(page);
    }
}

void bkModuleManager::moduleIsReady()
{
    if(p_activeModul.isNull()) return;
    p_activeModul->deactivate();
    p_activeModul->setState("strength", 28);
    p_activeModul = NULL;
}

void bkModuleManager::moduleHasJobFor(const QString& module, const QString& job, const int& mode)
{
    foreach(QPointer<bkModule> modul, m_modules) {
        if(modul->name() == module) {
            modul->job(job, mode);
        }
    }
}

void bkModuleManager::toggle(const QString module, const bool soll)
{
    foreach(QPointer<bkModule> modul, m_modules) {
        if(module == modul->name()) {
            modul->setState("enable", soll);
            qDebug() << "bkModuleManager::toggle" << module << soll;
        }
    }
}

QVariant bkModuleManager::state(const QString module, const QString key)
{
    QVariant ret;

    foreach(QPointer<bkModule> modul, m_modules) {
        if(modul->name() == module) {
            ret = modul->state(key).toVariant();
        }
    }

    return ret;
}
