#include "bkaccount.h"

#include <QNetworkReply>
#include <QWebPage>
#include <QWebElement>
#include <QTimer>

#include <QDebug>



bkAccount::bkAccount(bkWorld* world, int player_id, QJsonObject defaults, QObject *parent) :
    QObject(parent),
    p_world(world),
    p_parser(new bkParser(this)),
    p_logic(new bkLogic(this)),
    p_knight(p_world->player(player_id)),
    p_moduleManager(new bkModuleManager(defaults,this))
{
    m_status = defaults;

    m_status.insert("timerLogic", (int)0);
    m_status.insert("timerLogicDuration", (int)(3600*24)); // Logic active
    m_status.insert("modusLogic", (int)w_nothing);

    m_status.insert("timerCooldown", (int)0);
    m_status.insert("timerCooldownDuration", (int)(3600*24)); // Cooldown active
    m_status.insert("modusCooldown", (int)cd_unknown);

    m_status.insert("timerKarma", (int)0);
    m_status.insert("timerKarmaDuration", (int)(3600*24)); // Karma active
    m_status.insert("modusKarma", (int)0);

    m_status.insert("timerTreasury", (int)0);
    m_status.insert("timerTreasuryDuration", (int)(3600*6)); // 6 Stunden
    m_status.insert("modusTreasury", (int)0);

    m_status.insert("timerTurnier", (int)0);
    m_status.insert("timerTurnierDuration", (int)(3600*24)); // Vorbereitungszeit
    m_status.insert("modusTurnier", (int)0);

    m_status.insert("timerClanwar", (int)0);
    m_status.insert("timerClanwarDuration", (int)(3600*24)); // Vorbereitungszeit
    m_status.insert("modusClanwar", (int)cw_unknown);

    m_status.insert("location", QString());
    m_status.insert("missionPoints", (int)0);
    m_status.insert("timerMP", (int)0);

    m_status.insert("groupMissionPoints", (int)0);
    m_status.insert("timerGM", (int)0);
    m_status.insert("modusGM", (int)gm_unknown);

    m_status.insert("currentProposal", (int)0);

    connect(&t_mpTimer, SIGNAL(timeout()), this, SLOT(missionTimer()));
    connect(&t_gmTimer, SIGNAL(timeout()), this, SLOT(gmTimer()));

    adjustMissionTimer();
    adjustGMTimer();
    //qDebug() << QJsonDocument(m_status).toJson().data();
}

bkAccount::~bkAccount()
{
    delete p_moduleManager;
    delete p_logic;
    delete p_parser;
}

QVariant bkAccount::module(const QString& key)
{
    QVariant ret;
    if(m_status.keys().contains(key)) {
        ret = m_status.value(key).toVariant();
    }
    return ret;
}

QJsonObject bkAccount::saveState()
{
    QJsonObject state;
    //state.insert("config", m_config);
    //state.insert("status", m_status);
    state.insert("status", p_moduleManager->saveState());
    return state;
}

void bkAccount::setStatus(const QString& module, const QString& key, QVariant& data)
{
    qDebug() << "bkAccount::setStatus(mod,key,data)" << module << key << data;
}

void bkAccount::restoreState(QJsonObject state)
{
    QJsonObject c = state.value("config").toObject();
    foreach(QString key, c.keys()) {
        toggle(key, c.value(key).toBool());
    }
    QJsonObject s = state.value("status").toObject();
    //qDebug() << "bkAccount::restoreState" << QJsonDocument(s).toJson().data();
    p_moduleManager->restoreState(s);

    foreach(QString key, s.keys()) {
        QVariant value;
        if(key == "timerMP" || key == "timerGM" || key == "currentProposal") continue;
        if(s.value(key).isObject()) {
            value = s.value(key).toObject();
            setStatus(key, value);
            qDebug() << "bkAccount::restoreState" << QJsonDocument(s.value(key).toObject()).toJson().data();
            continue;
        } else if(s.value(key).isDouble()) {
            value = s.value(key).toDouble();
        } else if(s.value(key).isString()) {
            value = s.value(key).toString();
        }

        if(key.startsWith("timer") && (!key.endsWith("Duration"))) {
            QDateTime now = QDateTime::currentDateTimeUtc();
            QDateTime eta = QDateTime::fromTime_t(value.toInt());
            if(eta > now) {
                value = now.secsTo(eta);
            } else {
                value = 0;
            }
        }
        setStatus(key, value);
    }
}

QVariant bkAccount::status(const QString key)
{
    QVariant ret;

    if(!m_status.keys().contains(key)) return ret;
    if(key.startsWith("timer") && (!key.endsWith("Duration"))) {
        QDateTime now = QDateTime::currentDateTimeUtc();
        QDateTime eta = QDateTime::fromTime_t(m_status.value(key).toInt());
        if(eta > now) {
            ret = now.secsTo(eta);
            //qDebug() << "bkAccount::status" << key << ret << eta.toTime_t();
        } else {
            // rounds up?
            if(key == "timerClanwar") {
            }
        }
    } else {
        ret = m_status.value(key).toVariant();
    }

    return ret;
}

QVariant bkAccount::logicStrength(const QString& topic)
{
    return p_logic->status("strength" + topic);
}

// every hour
void bkAccount::missionTimer()
{
    int ist = status("missionPoints").toInt();
    int maxPoints = 120;
    int count = 5;
    QVariant prem = p_knight->value("manor_royalty");
    if(prem.isValid()) {
        maxPoints = 240;
        count = 10;
    }
    if(ist < maxPoints) {
        ist += count;
        m_status.insert("missionPoints", QJsonValue(ist));
    }
    // ToDo:
    // MPUpdate aufrufen?
    QWebPage* page = currentPage();
    page->mainFrame()->evaluateJavaScript("if(typeof MPUpdate === 'function') MPUpdate();");
    qDebug() << "bkAccount::missionTimer(PremiumAccount)" << prem << ist;
}

// every hour
void bkAccount::gmTimer()
{
    int ist = status("groupMissionPoints").toInt();
    int maxPoints = 120;
    if(ist < maxPoints) {
        ist += 5;
        m_status.insert("groupMissionPoints", QJsonValue(ist));
    }
    // ToDo:
    // GMUpdate aufrufen?
    QWebPage* page = currentPage();
    page->mainFrame()->evaluateJavaScript("if(typeof GMUpdate === 'function') GMUpdate();");
    qDebug() << "bkAccount::gmTimer()" << ist;
}

void bkAccount::adjustMissionTimer()
{
    if(t_mpTimer.isActive()) t_mpTimer.stop();
    QDateTime now = QDateTime::currentDateTimeUtc();
    m_status.insert("timerMP", QJsonValue((int)now.addSecs(3600).toTime_t()));
    //qDebug() << "adjusted timerMP here!" << m_status.value("timerMP").toInt();
    t_mpTimer.start(3600 * 1000);// 1h
}

void bkAccount::adjustGMTimer()
{
    if(t_gmTimer.isActive()) t_gmTimer.stop();
    QDateTime now = QDateTime::currentDateTimeUtc();
    m_status.insert("timerGM", QJsonValue((int)now.addSecs(3600).toTime_t()));
    //qDebug() << "adjusted timerGM here!" << m_status.value("timerMP").toInt();
    t_gmTimer.start(3600 * 1000);// 1h
}

void bkAccount::setStatus(const QString& key, QVariant& data)
{
    if(!m_status.keys().contains(key)) return;
    QString module, topic;
    if(key.startsWith("timer") && !key.endsWith("Duration")) {
        module = key.mid(5);
        topic = "timer";
    } else if(key.endsWith("Duration")) {
        module = key.left(key.length() - 8);
        module = module.mid(5);
        topic = "duration";
    } else if(key.startsWith("modus")) {
        module = key.mid(5);
        topic = "state";
    }

    if(key.startsWith("timer") && !key.endsWith("Duration")) {
        QDateTime now = QDateTime::currentDateTimeUtc();
        int count = data.toInt();
        if(count > 0) {
            m_status.insert(key, QJsonValue((int)now.addSecs(count).toTime_t()));
        } else {
            m_status.insert(key, QJsonValue((int)now.toTime_t()));
        }
    } else if(key == "missionPoints") {
        if(status(key).toInt() < data.toInt()) {
            adjustMissionTimer();
        }
        m_status.insert(key, QJsonValue::fromVariant(data));
    } else if(key == "groupMissionPoints") {
        if(status(key).toInt() < data.toInt()) {
            adjustGMTimer();
        }
        m_status.insert(key, QJsonValue::fromVariant(data));
    } else if(key == "currentProposal") {
        if(status(key).toInt() != data.toInt()) {
            m_status.insert(key, QJsonValue::fromVariant(data));
            p_logic->checkProposal();
        }
    } else {
        m_status.insert(key, QJsonValue::fromVariant(data));
    }
    //qDebug() << "--> bkAccount::setStatus" << module << topic << key << m_status.value(key).toInt();
}

void bkAccount::replyFinished(QNetworkReply* reply)
{
    //if(!m_config.value("enableAccount").toBool()) return;
    p_parser->replyFinished(reply);
    p_moduleManager->replyFinished(reply);
}

void bkAccount::toggle(const QString option, const bool soll)
{
    QString key;
    if(option.startsWith("enable")) {
        key = option.mid(6);
        if(m_status.keys().contains(key)) {
            QJsonObject mod = m_status.value(key).toObject();
            bool ist = mod.value("enable").toBool();
            if(ist != soll) {
                mod.insert("enable", soll);
                m_status.insert(key, mod);
                //ToDo:
                // Ui::update
                qDebug() << "bkAccount::toggle" << key << ist << soll;
            }
        }
    }
}

bool bkAccount::enabled(const QString& key)
{
    if(m_status.keys().contains(key)) {
        QJsonObject mod = m_status.value(key).toObject();
        if(mod.contains("enable")) {
            return mod.value("enable").toBool();
        }
    }
    return false;
}

QWebPage* bkAccount::currentPage()
{
    if(m_currentPage.isNull()) {
        foreach(QPointer<QWebPage> page, m_webPages) {
            if(!page.isNull()) return page;
        }
    }
    return m_currentPage;
}

void bkAccount::loadUrl(const QUrl& l_url)
{
    QWebPage* workPage = Q_NULLPTR;
    if(m_currentPage.isNull()) {
        foreach(QPointer<QWebPage> page, m_webPages) {
            if(page.isNull()) continue;
            workPage = page;
            break;
        }
    } else {
        workPage = m_currentPage;
    }
    if(workPage == Q_NULLPTR) return;
    QUrl url = workPage->mainFrame()->baseUrl();
    workPage->mainFrame()->load(l_url);
}

void bkAccount::loadFinished(QWebPage* page)
{
    QStringList paths = page->mainFrame()->url().path().split("/", QString::SkipEmptyParts);
    if(!paths.count()) return; // nothing to do

    for(int i = m_webPages.count();i > 0; --i) {
        if(m_webPages.at(i-1).isNull()) m_webPages.removeAt(i-1);
    }
    if(!m_webPages.contains(page)) {
        m_webPages.append(page);
    }

    page->mainFrame()->evaluateJavaScript("window.onerror = null;");
    p_moduleManager->loadFinished(page);

    QWebElement document = page->mainFrame()->documentElement();

    p_parser->loadFinished(page);

    m_currentPage = page;
    p_logic->loadFinished(page);
}

QDebug operator << (QDebug dbg, const bkAccount* account)
{
    dbg.space() << account->player()->toJson().data();
    dbg.space() << QJsonDocument(account->m_status).toJson().data();
    return dbg.maybeSpace();
}
