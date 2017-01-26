#include "plugin.h"

#include <QFile>
#include <QTime>
#include <QWebFrame>
#include <QWebElement>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QtGui/QDesktopServices>

#include <QDebug>


Plugin::~Plugin()
{
    Q_CLEANUP_RESOURCE(data);
}

void Plugin::loadSettings(QSettings& settings)
{
    settings.beginGroup(name());

    m_pluginSettings.enabled = settings.value(QLatin1String("enabled"), true).toBool();
    m_pluginSettings.templatePath = settings.value(QLatin1String("templatePath"),
                        QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + name().toLower() + QLatin1String("/template/")).toString();
    m_pluginSettings.urlPatterns = settings.value(QLatin1String("patterns"),
                        QStringList("daimonmicha.bplaced.net")).toStringList();

    settings.endGroup();

    qDebug() << "\t"+name()+"::loadSettings" << m_pluginSettings.urlPatterns;
}

void Plugin::saveSettings(QSettings& settings)
{
    settings.beginGroup(name());

    settings.setValue(QLatin1String("enabled"), m_pluginSettings.enabled);
    settings.setValue(QLatin1String("templatePath"), m_pluginSettings.templatePath);
    settings.setValue(QLatin1String("patterns"), m_pluginSettings.urlPatterns);

    settings.endGroup();

    qDebug() << "\t"+name()+"::saveSettings";
}

void Plugin::saveState(QSettings& settings)
{
    settings.beginGroup(name());

    settings.endGroup();

//    qDebug() << "\t"+name()+"::saveState";
}

Account *Plugin::accFromCookie(const QString cValue)
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

void Plugin::injectHtml(QWebFrame* mainFrame, Account*)
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

void Plugin::replyFinished(QNetworkReply* reply)
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
    Account *current = accFromCookie(QString(cValue));

    current->replyFinished(reply);

    QString logString;
    QDateTime now = QDateTime::currentDateTimeUtc();
    logString.append(now.toString("[yyyy-MM-dd HH:mm:ss]"));
    logString.append("  "+name()+"::replyFinished (" + url.path());
    logString.append(")");
    QByteArray post = reply->property("postData").toByteArray();
    if(post.length() > 0) logString.append(", POST:'"+post+"'");
    qDebug() << logString;
}

void Plugin::loadFinished(QWebEnginePage* page)
{
    QUrl url = page->url();

    QList<QNetworkCookie> cookies = page->networkAccessManager()->cookieJar()->cookiesForUrl(url);
    QByteArray cValue;
    if(!cookies.count()) return;
    cValue = cookies.at(0).value();
    if(cValue.isEmpty()) return;

    // look for an account with that cookie
    Account *current = accFromCookie(QString(cValue));

    page->mainFrame()->addToJavaScriptWindowObject("account", current);

    injectHtml(page->mainFrame(), current);

    current->loadFinished(page);

    QString logString;
    QDateTime now = QDateTime::currentDateTimeUtc();
    logString.append(now.toString("[yyyy-MM-dd HH:mm:ss]"));
    logString.append("  "+name()+"::loadFinished (" + url.path());
    logString.append(") '" + page->mainFrame()->title() + "'");
    qDebug() << logString;
}
