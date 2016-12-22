#include "plugin.h"

#include <QFile>
#include <QTime>
#include <QWebFrame>
#include <QWebElement>
#include <QNetworkCookie>
#include <QNetworkCookieJar>

#include <QDebug>


Plugin::~Plugin()
{
    Q_CLEANUP_RESOURCE(data);
}

bool Plugin::isMyUrl(const QUrl &url) const
{
    QString host = url.host();
    if(host.endsWith(".daimonmicha.bplaced.net")) return(true);
    return(false);
}

void Plugin::loadSettings(QSettings &settings)
{
    Q_INIT_RESOURCE(data);

    // Create seed for the random
    // That is needed only once on application startup
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    settings.beginGroup(QLatin1String("plugin"));
    settings.endGroup();

    qDebug() << "\tPlugin::loadSettings";
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
    QString path;
    //path = qApp->applicationDirPath() + "/plugins/battleknight/htmls/";
    // path = "/home/micha/.local/share/DaimonNetworks/webkitBrowser";
    path = "/home/micha/Projekte/gameBrowser/plugins/plugin/htmls/";
    QFile inject;
    inject.setFileName(path + file);
    if(!inject.open(QIODevice::ReadOnly)) {
        inject.setFileName(":/plugin/" + file);
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

    QString di;
    if(readDataFile("inject.html", di) <= 0) return;

    QWebElement body = mainFrame->findFirstElement("body");
    body.appendInside(di);

    if(account->isActive("account")) {
        QWebElement checker = body.findFirst("#clickChecker");
        if(!checker.isNull()) checker.setAttribute("checked", "checked");
    }

    di.truncate(0);
    if(readDataFile("checkscript.js", di) <= 0) return;
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
