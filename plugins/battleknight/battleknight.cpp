#include "battleknight.h"

#include <QRegExp>
#include <QFile>
#include <QTime>
#include <QWebFrame>
#include <QWebElement>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QtGui/QDesktopServices>

#include <QDebug>



jsConsole::jsConsole(QObject *parent) :
    QObject(parent)
{
}

void jsConsole::log(const QByteArray& data)
{
    qDebug() << "jsConsole::log:" << data;
}





BattleKnight::~BattleKnight()
{
    if(m_accounts.count() > 0) foreach(Account *account, m_accounts) {
        delete(account);
    }
    Q_CLEANUP_RESOURCE(data);
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

    if(m_accounts.count() > 0) foreach(Account *account, m_accounts) {
        if(!account->fingerprint().isEmpty()) {
            m_accountStates.insert(account->fingerprint(), account->state());
            //debugMsg.append("saveState for \""+m_accountStates.insert(account->fingerprint(), account->state()).key()+"\".\n");
        }
    }
    QJsonDocument ret(m_accountStates);

    settings.setValue(QLatin1String("accountStates"), ret.toJson(QJsonDocument::Compact).constData());
    settings.endGroup();

    //debugMsg.append(name()+"::saveStates: \n").append(ret.toJson(QJsonDocument::Compact)+"\n"); // QJsonDocument::Compact
    if(!debugMsg.isEmpty()) qDebug() << debugMsg.constData();
}

void BattleKnight::hasPlayer()
{
    Account* account = qobject_cast<Account *>(sender());
    if(!account) return;
    // restoreState versuchen
    if(m_accountStates.contains(account->fingerprint())) {
        account->restoreState(m_accountStates.value(account->fingerprint()).toObject());
    }
    //qDebug() << "BattleKnight::hasPlayer" << account->fingerprint();
}

Account *BattleKnight::accFromCookie(const QString cValue, const QUrl url)
{
    Account *ret = NULL;
    if(m_accounts.count() > 0) foreach(Account *account, m_accounts) {
        if(account->cookieValue() == cValue) {
            ret = account;
            break;
        }
    }
    if(ret == NULL) {
        ret = new Account(cValue, url, this);
        ret->toggle("enablePlugin", m_pluginSettings.enabled);
        connect(ret, SIGNAL(playerFound()), this, SLOT(hasPlayer()));
        m_accounts.append(ret);
    }
    return(ret);
}

void BattleKnight::loadFinished(QWebPage* page)
{
    QUrl url = page->mainFrame()->url();

    QList<QNetworkCookie> cookies = page->networkAccessManager()->cookieJar()->cookiesForUrl(url);
    QByteArray cValue;
    if(!cookies.count()) return;
    cValue = cookies.at(0).value();
    if(cValue.isEmpty()) return;

    // look for an account with that cookie
    Account *current = accFromCookie(QString(cValue), url);

    injectHtml(page->mainFrame(), current);

    page->mainFrame()->addToJavaScriptWindowObject("account", current);
    page->mainFrame()->addToJavaScriptWindowObject("console",&m_console);

    current->loadFinished(page);

    QString logString;
    QDateTime now = QDateTime::currentDateTimeUtc();
    logString.append(now.toString("[yyyy-MM-dd HH:mm:ss]"));
    logString.append("  "+name()+"::loadFinished (" + url.path());
    logString.append(") '" + page->mainFrame()->title() + "'");
    qDebug() << logString;
}

void BattleKnight::replyFinished(QNetworkReply* reply)
{
    QUrl url = reply->url();
    QString path = url.path();
    if(m_excludeExtensions.contains(path.mid(path.lastIndexOf(".") + 1),Qt::CaseInsensitive)) return;

    QList<QNetworkCookie> cookies = reply->manager()->cookieJar()->cookiesForUrl(url);
    QByteArray cValue;
    if(!cookies.count()) return;
    cValue = cookies.at(0).value();
    if(cValue.isEmpty()) return;

    // look for an account with that cookie
    Account *current = accFromCookie(QString(cValue), url);

    current->replyFinished(reply);

    QString logString;
    QDateTime now = QDateTime::currentDateTimeUtc();
    logString.append(now.toString("[yyyy-MM-dd HH:mm:ss]"));
    logString.append("  "+name()+"::replyFinished (" + url.path());
    logString.append(")");
    if(url.query().length() > 0) logString.append(",\n\t  GET:'"+url.query()+"'");
    QByteArray post = reply->property("postData").toByteArray();
    if(post.length() > 0) logString.append(",\n\t POST:'"+post+"'");
    qDebug() << logString;
}

void BattleKnight::injectHtml(QWebFrame* mainFrame, Account*)
{
    QWebElement pluginDiv = mainFrame->findFirstElement("#accountPlugin");
    if(!pluginDiv.isNull()) return;

    QWebElement body = mainFrame->findFirstElement("body");
    QString di;

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
    }

    if(readDataFile("inject.css", di) <= 0) {
        return;
    }
    body.appendInside(di);

    if(readDataFile("inject.html", di) <= 0) {
        return;
    }
    body.appendInside(di);

    if(readDataFile("gamescript.js", di) <= 0) {
        return;
    }
    mainFrame->evaluateJavaScript(di);
}
