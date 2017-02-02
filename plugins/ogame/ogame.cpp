#include "ogame.h"

#include <QFile>
#include <QTime>
#include <QWebFrame>
#include <QWebElement>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QUrlQuery>
#include <QtGui/QDesktopServices>
#include <QTimer>

#include <QDebug>


OGame::~OGame()
{
    Q_CLEANUP_RESOURCE(data);
}

void OGame::loadSettings(QSettings& settings)
{
    settings.beginGroup(name());

    m_pluginSettings.enabled = settings.value(QLatin1String("enabled"), true).toBool();
    m_pluginSettings.templatePath = settings.value(QLatin1String("templatePath"),
                        QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + name().toLower() + QLatin1String("/template/")).toString();
    m_pluginSettings.urlPatterns = settings.value(QLatin1String("patterns"),
                        QStringList("daimonmicha.bplaced.net")).toStringList();

    settings.endGroup();

    qDebug() << "\t" << name()+"::loadSettings" << m_pluginSettings.urlPatterns;
}

void OGame::saveSettings(QSettings& settings)
{
    settings.beginGroup(name());

    settings.setValue(QLatin1String("enabled"), m_pluginSettings.enabled);
    settings.setValue(QLatin1String("templatePath"), m_pluginSettings.templatePath);
    settings.setValue(QLatin1String("patterns"), m_pluginSettings.urlPatterns);

    settings.endGroup();

    qDebug() << "\t" << name()+"::saveSettings";
}

void OGame::saveState(QSettings& settings)
{
    settings.beginGroup(name());

    settings.endGroup();

//    qDebug() << "\t"+name()+"::saveState";
}

Account *OGame::accFromCookie(const QString cValue)
{
    Account *ret = NULL;
    if(m_accounts.count() > 0) foreach(Account *account, m_accounts) {
        if(account->cookieValue() == cValue) {
            ret = account;
            break;
        }
    }
    if(ret == NULL) {
        ret = new Account(cValue, this);
        ret->toggle("enableAccount", m_pluginSettings.enabled);
        m_accounts.append(ret);
    }
    return(ret);
}

void OGame::injectHtml(QWebFrame* mainFrame, Account*)
{
    if(!mainFrame->findFirstElement("#accountPlugin").isNull()) return;

    QWebElement body = mainFrame->findFirstElement("body");
    QString di;

    di.truncate(0);
    if(readDataFile("inject.css", di) <= 0) {
        return;
    }
    body.appendInside(di);

    di.truncate(0);
    if(readDataFile("inject.html", di) <= 0) {
        return;
    }
    body.appendInside(di);

    di.truncate(0);
    if(readDataFile("gamescript.js", di) <= 0) {
        return;
    }
    mainFrame->evaluateJavaScript(di);
}

void OGame::replyFinished(QNetworkReply* reply)
{
    if(!s_networkManager) s_networkManager = reply->manager();
    QUrl url = reply->url();
    QString path = url.path();
    if(m_excludeExtensions.contains(path.mid(path.lastIndexOf(".") + 1),Qt::CaseInsensitive)) return;

    QList<QNetworkCookie> cookies = s_networkManager->cookieJar()->cookiesForUrl(url);
    if(!cookies.count()) return;
    QByteArray cValue;
    foreach(QNetworkCookie cookie, cookies) {
        if(cookie.name().startsWith("login_")) {
            cValue = cookie.value();
        }
    }

    if(cValue.isEmpty()) return;
    // look for an account with that cookie
    Account *current = accFromCookie(QString(cValue));
    current->replyFinished(reply);

    QStringList paths = url.path().split("/",QString::SkipEmptyParts);
    if(paths.count() == 2 && paths.at(0) == QString("main")) {
        if(paths.at(1) == QString("login")) {
            QUrlQuery q(reply->property("postData").toString());
            qDebug() << "\t Login:" << q.toString();
        }
    }

    QString logString;
    QDateTime now = QDateTime::currentDateTimeUtc();
    logString.append(now.toString("[yyyy-MM-dd HH:mm:ss]"));
    logString.append("  "+name()+"::replyFinished (" + url.path());
    if(!url.query().isEmpty()) logString.append("?" + url.query());
    logString.append(")");
    QByteArray post = reply->property("postData").toByteArray();
    if(post.length() > 0) logString.append(", POST:'"+post+"'");
    qDebug() << logString;
}

QByteArray OGame::lastServerLogin(QUrl & url)
{
    QList<QNetworkCookie> cookies = s_networkManager->cookieJar()->cookiesForUrl(url);
    foreach(QNetworkCookie cookie, cookies) {
        if(cookie.name().startsWith("login_")) {
            QByteArray value = QByteArray::fromPercentEncoding(cookie.value());
            return(value.split(':').at(1));
        }
    }
    return(QByteArray());
}

void OGame::loadFinished(QWebPage* page)
{
    if(!s_networkManager) s_networkManager = page->networkAccessManager();
    QUrl url = page->mainFrame()->url();

    QList<QNetworkCookie> cookies = s_networkManager->cookieJar()->cookiesForUrl(url);
    if(!cookies.count()) return;
    QByteArray cValue;

    foreach(QNetworkCookie cookie, cookies) {
        if(cookie.name() == "OG_lastServer") {
            url.setHost(cookie.value());
            QByteArray value = lastServerLogin(url);
            if(value.isEmpty()) return;
            QWebElement input = page->mainFrame()->findFirstElement("#usernameLogin");
            input.setAttribute("value",value);
            page->mainFrame()->evaluateJavaScript("document.getElementById('passwordLogin').focus();");
            // password setzen & submit klicken geht natürlich auch ;-)
            return;
        }
        if(cookie.name().startsWith("login_")) {
            cValue = cookie.value();
        }
    }

    if(cValue.isEmpty()) return;

    // look for an account with that cookie
    Account *current = accFromCookie(QString(cValue));

    injectHtml(page->mainFrame(), current);

    page->mainFrame()->addToJavaScriptWindowObject("account", current);

    current->loadFinished(page);

    QString logString;
    QDateTime now = QDateTime::currentDateTimeUtc();
    logString.append(now.toString("[yyyy-MM-dd HH:mm:ss]"));
    logString.append("  "+name()+"::loadFinished (" + url.path());
    logString.append(") '" + page->mainFrame()->title() + "'");
    qDebug() << logString;
}
