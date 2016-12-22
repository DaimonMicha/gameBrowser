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

bool Plugin::isMyUrl(const QUrl &url) const
{
    QString host = url.host();

    foreach(QString pattern, m_settings.urlPatterns) {
        if(host.endsWith(pattern)) return(true);
    }

    return(false);
}

void Plugin::loadSettings(QSettings &settings)
{
    Q_INIT_RESOURCE(data);

    // Create seed for the random
    // That is needed only once on application startup
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    m_excludeExtensions
            << "js"
            << "mp3"
            << "ogg"
            << "css"
            << "cur"
            << "ico"
            << "gif"
            << "png"
            << "jpg"
            << "swf"
               ;

    settings.beginGroup(name());

    m_settings.enabled = settings.value(QLatin1String("enabled"), true).toBool();
    m_settings.templatePath = settings.value(QLatin1String("templatePath"),
                        QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1String("/template")).toString();
    m_settings.urlPatterns = settings.value(QLatin1String("patterns"),
                        QStringList("daimonmicha.bplaced.net")).toStringList();

    settings.endGroup();

    qDebug() << "\tPlugin::loadSettings" << m_settings.urlPatterns;
}

void Plugin::saveSettings(QSettings &)
{
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

    return(ret);
}

int Plugin::readDataFile(const QString file, QString& data)
{
    QFile inject;
    inject.setFileName(m_settings.templatePath + file);
    if(!inject.open(QIODevice::ReadOnly)) {
        inject.setFileName(":/"+name().toLower()+"/" + file);
        if(!inject.open(QIODevice::ReadOnly)) {
            return(-1);
        }
    }
    if(inject.isOpen()) {
        QByteArray bytes = inject.readAll();
        //qDebug() << "[Plugin::readDataFile]:" << inject.fileName();
        inject.close();
        data.append(bytes);
        return(data.length());
    }

    return(-1);
}

void Plugin::injectHtml(QWebFrame* mainFrame, Account* account)
{
    QWebElement pluginDiv = mainFrame->findFirstElement("#accountPlugin");
    if(!pluginDiv.isNull()) return;

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

    if(account->isActive("account")) {
        QWebElement checker = body.findFirst("#clickChecker");
        if(!checker.isNull()) checker.setAttribute("checked", "checked");
    }

    di.truncate(0);
    if(readDataFile("checkscript.js", di) <= 0) {
        return;
    }
    mainFrame->evaluateJavaScript(di);
}

void Plugin::replyFinished(QNetworkReply* reply)
{
    QUrl url = reply->url();
    if(!isMyUrl(url)) return;
}

void Plugin::loadFinished(QWebPage* page)
{
    QUrl url = page->mainFrame()->url();
    if(!isMyUrl(url)) return;

    QList<QNetworkCookie> cookies = page->networkAccessManager()->cookieJar()->cookiesForUrl(url);
    QByteArray cValue;
    if(!cookies.count()) return;
    cValue = cookies.at(0).value();
    if(cValue.isEmpty()) return;

    // look for an account with this cookie
    Account *current = accFromCookie(QString(cValue));
    if(current == NULL) {
        current = new Account(cValue);
        m_accounts.append(current);
    }

    page->mainFrame()->addToJavaScriptWindowObject("account", current);
    current->loadFinished(page);
    injectHtml(page->mainFrame(), current);

    QString logString;
    QDateTime now = QDateTime::currentDateTimeUtc();
    logString.append(now.toString("[yyyy-MM-dd HH:mm:ss]"));
    logString.append("  Plugin::loadFinished (" + url.path());
    logString.append(") '" + page->mainFrame()->title() + "'");
    qDebug() << logString;
}
