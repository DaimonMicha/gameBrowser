#include "bkaccount.h"

#include <QNetworkReply>
#include <QWebPage>
#include <QWebElement>
#include <QTimer>

#include <QDebug>



bkAccount::bkAccount(bkWorld* world, int player_id, QJsonObject defaults, QObject *parent) :
    QObject(parent),
    p_world(world),
    p_knight(p_world->player(player_id)),
    p_moduleManager(new bkModuleManager(defaults,this))
{
    m_status = defaults;
    //qDebug() << QJsonDocument(m_status).toJson().data();
}

bkAccount::~bkAccount()
{
    delete p_moduleManager;
}

QVariant bkAccount::module(const QString& key)
{
    QVariant ret;
    QJsonObject modules = p_moduleManager->saveState();
    if(modules.keys().contains(key)) {
        ret = modules.value(key).toVariant();
    }

    if(ret.isNull() && m_status.keys().contains(key)) {
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

void bkAccount::restoreState(QJsonObject state)
{
    QJsonObject c = state.value("config").toObject();
    foreach(QString key, c.keys()) {
        toggle(key, c.value(key).toBool());
    }
    QJsonObject s = state.value("status").toObject();
    //qDebug() << "bkAccount::restoreState" << QJsonDocument(s).toJson().data();
    p_moduleManager->restoreState(s);
}

QVariant bkAccount::state(const QString module, const QString key)
{
    QVariant ret;

    ret = p_moduleManager->state(module, key);

    //qDebug() << "bkAccount::state" << module << key << ret;

    return ret;
}

void bkAccount::setState(const QString& module, const QString& key, QJsonValue value)
{
    qDebug() << "bkAccount::setStatus(mod,key,data)" << module << key << value;
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
            //adjustMissionTimer();
        }
        m_status.insert(key, QJsonValue::fromVariant(data));
    } else if(key == "groupMissionPoints") {
        if(status(key).toInt() < data.toInt()) {
            //adjustGMTimer();
        }
        m_status.insert(key, QJsonValue::fromVariant(data));
    } else if(key == "currentProposal") {
        if(status(key).toInt() != data.toInt()) {
            m_status.insert(key, QJsonValue::fromVariant(data));
            //p_logic->checkProposal();
        }
    } else {
        m_status.insert(key, QJsonValue::fromVariant(data));
    }
    //qDebug() << "--> bkAccount::setStatus" << module << topic << key << m_status.value(key).toInt();
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

void bkAccount::replyFinished(QNetworkReply* reply)
{
    //if(!m_config.value("enableAccount").toBool()) return;

    //p_parser->replyFinished(reply);

    p_moduleManager->replyFinished(reply);
}

void bkAccount::toggle(const QString option, const bool soll)
{
    QString key;
    if(option.startsWith("enable")) {
        key = option.mid(6);
        p_moduleManager->toggle(key, soll);
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
    //QUrl url = workPage->mainFrame()->baseUrl();
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

    m_currentPage = page;
}

QDebug operator << (QDebug dbg, const bkAccount* account)
{
    dbg.space() << "bkAccount:" << account->player()->toJson().data();
    dbg.space() << QJsonDocument(account->m_status).toJson().data();
    return dbg.maybeSpace();
}
