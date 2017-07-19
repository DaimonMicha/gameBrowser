#include "battleknight.h"

#include "battleknightdock.h"

#include "bkworld.h"
#include "bkaccount.h"

#include <QRegExp>
#include <QFile>
#include <QTime>
#include <QWebFrame>
#include <QWebElement>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QtGui/QDesktopServices>

#include <QDebug>





BattleKnight::BattleKnight() :
    PluginInterface()
{
    s_dockWidget = new QToolBox();
    QString di;
    if(readDataFile("modules.json", di) > 0) {
        m_moduleDefaults = QJsonDocument::fromJson(di.toLocal8Bit()).object();
        //qDebug() << QJsonDocument(m_moduleDefaults).toJson().data();
        //accWorld->setLocationStrings(di);
    }

    foreach(QString key, m_moduleDefaults.keys()) {
        QJsonObject mod = m_moduleDefaults.value(key).toObject();
        //qDebug() << key;
        if(key == "Account") {
            m_modules.append(new modAccount(key, mod.value("title").toString(), mod.value("tooltip").toString()));
        } else if(key == "Missions") {
            m_modules.append(new modMissions(key, mod.value("title").toString(), mod.value("tooltip").toString()));
        } else if(key == "GM") {
            m_modules.append(new modGM(key, mod.value("title").toString(), mod.value("tooltip").toString()));
        } else if(key == "Duels") {
            m_modules.append(new modDuel(key, mod.value("title").toString(), mod.value("tooltip").toString()));
        } else {
            m_modules.append(new accModule(key, mod.value("title").toString(), mod.value("tooltip").toString()));
        }
    }
/*
    m_modules.append(new accModule("Work", "Arbeiten"));
    m_modules.append(new accModule("Clanwar", "Ordensschlacht"));
    m_modules.append(new accModule("Turnier", "Turnier"));
    m_modules.append(new accModule("Treasury", "Schatzkammer"));
*/
}

BattleKnight::~BattleKnight()
{
    while(!m_modules.isEmpty()) {
        m_modules.takeLast()->deleteLater();
    }
    while(!m_accountList.isEmpty()) {
        m_accountList.takeLast()->deleteLater();
    }
    while(!m_worldList.isEmpty()) {
        m_worldList.takeLast()->deleteLater();
    }
    //qDebug() << "BattleKnight destroyed.";
}

void BattleKnight::loadSettings(QSettings &settings)
{
    settings.beginGroup(name());

    m_pluginSettings.enabled = settings.value(QLatin1String("enabled"), true).toBool();
    m_pluginSettings.templatePath = settings.value(QLatin1String("templatePath"),
                        QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + name().toLower() + QLatin1String("/template/")).toString();
    m_pluginSettings.urlPatterns = settings.value(QLatin1String("patterns"),
                        QStringList(".battleknight.gameforge.com")).toStringList();

    m_accountStates = QJsonDocument::fromJson(settings.value(QLatin1String("accountStates")).toByteArray()).object();

    settings.endGroup();

    //qDebug() << "\t"+name()+"::loadSettings";
}

void BattleKnight::saveSettings(QSettings& settings)
{
    settings.beginGroup(name());

    settings.setValue(QLatin1String("enabled"), m_pluginSettings.enabled);
    settings.setValue(QLatin1String("templatePath"), m_pluginSettings.templatePath);
    settings.setValue(QLatin1String("patterns"), m_pluginSettings.urlPatterns);

    settings.endGroup();

    //qDebug() << "\t"+name()+"::saveSettings";
}

void BattleKnight::saveState(QSettings& settings)
{
    QByteArray debugMsg;
    settings.beginGroup(name());

    foreach(bkAccount *acc, m_accountList) {
        QString key = QString("%1::%2").arg(acc->world()->name()).arg(acc->player()->id());
        m_accountStates.insert(key, acc->saveState());
    }

    settings.setValue(QLatin1String("accountStates"), QJsonDocument(m_accountStates).toJson(QJsonDocument::Compact).constData());
    settings.endGroup();

    //debugMsg.append(name()+"::saveStates: \n").append(QJsonDocument(m_accountStates).toJson(QJsonDocument::Compact)+"\n");
    if(!debugMsg.isEmpty()) qDebug() << debugMsg.constData();
}


QString BattleKnight::getCookie(const QUrl& url)
{
    if(s_networkManager == Q_NULLPTR) return QString();
    QList<QNetworkCookie> cookies = s_networkManager->cookieJar()->cookiesForUrl(url);
    foreach(QNetworkCookie cookie, cookies) {
        //qWarning() << cookie.name() << cookie.value();
        if(cookie.name() == "BattleKnight") {
            QString val = QUrl::fromPercentEncoding(cookie.value());
            return val;
        }
    }
    return QString();
}

bkAccount *BattleKnight::findAccount(const QUrl& url)
{
    QString c = getCookie(url);
    if(c.isEmpty()) return Q_NULLPTR;
    int knight_id = 0;
    if(c.contains('#')) knight_id = c.split('#').at(1).toInt();

    if(knight_id == 0) return Q_NULLPTR;

    QString w = url.host().split('.').at(0);

    foreach(bkAccount *a, m_accountList) {
        if((a->world()->name() == w) && (a->player()->id() == knight_id)) {
            return a;
        }
    }

    bkWorld *accWorld = Q_NULLPTR;
    foreach(bkWorld *world, m_worldList) {
        if(world->name() == w) {
            accWorld = world;
            break;
        }
    }
    if(accWorld == Q_NULLPTR) {
        accWorld = new bkWorld(w,this);
        QString di;
        if(readDataFile("locations.json", di) > 0) {
            accWorld->setLocationStrings(di);
        }
        if(readDataFile("karma.json", di) > 0) {
            accWorld->setKarmaStrings(di);
        }
        m_worldList.append(accWorld);
    }
    bkAccount *acc = new bkAccount(accWorld, knight_id, m_moduleDefaults, this);
    QString key = QString("%1::%2").arg(acc->world()->name()).arg(acc->player()->id());
    if(m_accountStates.contains(key)) {
        acc->restoreState(m_accountStates.value(key).toObject());
    }
    m_accountList.append(acc);
    return acc;
}

void BattleKnight::updateMP()
{
    for(int i = m_webPages.count() - 1;i >= 0; --i) {
        if(m_webPages.at(i).isNull()) m_webPages.removeAt(i);
        else {
            bkAccount* account = findAccount(m_webPages.at(i)->mainFrame()->url());
            if(account == Q_NULLPTR) {
                qDebug() << "BattleKnight::updateMP() no account found!";
                return;
            }
        }
    }
}

void BattleKnight::loadFinished(QWebPage* page)
{
    QUrl url = page->mainFrame()->url();
    if(s_networkManager == Q_NULLPTR) s_networkManager = page->networkAccessManager();

    // look for an account with that cookie
    bkAccount* account = findAccount(page->mainFrame()->url());
    if(account == Q_NULLPTR) {
        qDebug() << "BattleKnight::loadFinished() no account found!";
        return;
    }

    for(int i = m_webPages.count() - 1;i >= 0; --i) {
        if(m_webPages.at(i).isNull()) m_webPages.removeAt(i);
    }
    if(!m_webPages.contains(page)) {
        m_webPages.append(page);
    }

    page->mainFrame()->addToJavaScriptWindowObject("account", account);
    page->mainFrame()->evaluateJavaScript("window.onerror = null;");
    injectHtml(page->mainFrame());
    account->loadFinished(page);
    foreach(accModule *module, m_modules) {
        module->pageLoaded(account, page);
    }

    QString logString;
    QDateTime now = QDateTime::currentDateTimeUtc();
    logString.append(now.toString("[yyyy-MM-dd HH:mm:ss]"));
    logString.append("  "+name()+"::loadFinished (" + url.path());
    logString.append(") '" + page->mainFrame()->title() + "'");
    logString.append(QString(", %1 Tab(s).").arg(m_webPages.count()));
    qDebug() << logString.toLatin1().data();// << account;
}

void BattleKnight::replyFinished(QNetworkReply* reply)
{
    if(s_networkManager == Q_NULLPTR) s_networkManager = reply->manager();
    QUrl url = reply->url();
    QString path = url.path();

    if(m_excludeExtensions.contains(path.mid(path.lastIndexOf(".") + 1),Qt::CaseInsensitive)) return;

    //QList<QNetworkCookie> cookies = reply->manager()->cookieJar()->cookiesForUrl(url);
    //QString cValue = getCookie(url);
    //if(cValue.isEmpty()) return;

    // look for an account with that cookie
    bkAccount* account = findAccount(reply->url());
    if(account == Q_NULLPTR) {
        qDebug() << "BattleKnight::replyFinished() no account found!";
        return;
    }

    account->replyFinished(reply);

    QString logString;
    QDateTime now = QDateTime::currentDateTimeUtc();
    logString.append(now.toString("[yyyy-MM-dd HH:mm:ss]"));
    logString.append("  "+name()+"::replyFinished (" + url.path());
    logString.append(")");
    if(url.query().length() > 0) logString.append(",\n\t  GET:'"+url.query()+"'");
    QByteArray post = reply->property("postData").toByteArray();
    if(post.length() > 0) {
        logString.append(",\n\t POST:'"+post+"'");
        logString.append(QString("\n\t               %1").arg(now.toMSecsSinceEpoch()));
    }
    qDebug() << logString.toLatin1().data();
}

void BattleKnight::injectHtml(QWebFrame* mainFrame)
{
    QWebElement pluginDiv = mainFrame->findFirstElement("#accountPlugin");
    if(!pluginDiv.isNull()) return;

    QString di;
    QWebElement body = mainFrame->findFirstElement("body");

    if(body.classes().contains("nonPremium")) {
        QWebElement netBar = mainFrame->findFirstElement("#mmonetbar");
        netBar.removeFromDocument();
        netBar = mainFrame->findFirstElement("#networkBar");
        netBar.removeFromDocument();
        QWebElement div = body.findFirst("div");
        div.setStyleProperty("position","relative");
        div.setStyleProperty("top","-32px");
        QWebElement head = mainFrame->findFirstElement("head");
        div = head.findFirst("style");
        div.removeFromDocument();
    } else {
        bkAccount* account = findAccount(mainFrame->url());
        QJsonObject p;
        p.insert("manor_royalty", true);
        account->player()->setData(p);
    }

    if(readDataFile("inject.css", di) <= 0) {
        return;
    }
    body.appendInside(di);

    if(readDataFile("inject.html", di) <= 0) {
        return;
    }
    body.appendInside(di);

    if(readDataFile("locations.json", di) <= 0) {
        return;
    }
    mainFrame->evaluateJavaScript(di.prepend("var km_locations=").append(";"));

    if(readDataFile("karma.json", di) > 0) {
        mainFrame->evaluateJavaScript(di.prepend("var km_karma=").append(";"));
    }

    if(readDataFile("updater.js", di) > 0) {
        mainFrame->evaluateJavaScript(di);
        //qDebug() << di.toLocal8Bit().data();
    }

}
