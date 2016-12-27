#include "battleknight.h"

#include <QFile>
#include <QTime>
#include <QWebFrame>
#include <QWebElement>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QtGui/QDesktopServices>

#include <QJsonDocument>

#include <QDebug>


void BattleKnight::initPlugin()
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
            << "woff"
            << "swf"
               ;
}

BattleKnight::~BattleKnight()
{
    Q_CLEANUP_RESOURCE(data);
}

bool BattleKnight::isMyUrl(const QUrl &url) const
{
    QString host = url.host();

    foreach(QString pattern, m_settings.urlPatterns) {
        if(host.endsWith(pattern)) return(true);
    }

    return(false);
}

void BattleKnight::loadSettings(QSettings &settings)
{
    settings.beginGroup(name());

    m_settings.enabled = settings.value(QLatin1String("enabled"), true).toBool();
    m_settings.templatePath = settings.value(QLatin1String("templatePath"),
                        QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + name().toLower() + QLatin1String("/template/")).toString();
    m_settings.urlPatterns = settings.value(QLatin1String("patterns"),
                        QStringList(".battleknight.gameforge.com")).toStringList();

    settings.endGroup();

    qDebug() << "\t"+name()+"::loadSettings" << m_settings.urlPatterns;
}

void BattleKnight::saveSettings(QSettings& settings)
{
    settings.beginGroup(name());

    settings.setValue(QLatin1String("enabled"), m_settings.enabled);
    settings.setValue(QLatin1String("templatePath"), m_settings.templatePath);
    settings.setValue(QLatin1String("patterns"), m_settings.urlPatterns);

    settings.endGroup();

    qDebug() << "\t"+name()+"::saveSettings";
}

Account *BattleKnight::accFromCookie(const QString cValue)
{
    Account *ret = NULL;
    if(m_accounts.count() > 0) foreach (Account *account, m_accounts) {
        if(account->cookieValue() == cValue) {
            ret = account;
            break;
        }
    }
    if(ret == NULL) {
        ret = new Account(cValue);
        ret->toggle("account", m_settings.enabled);
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

int BattleKnight::readDataFile(const QString file, QString& data)
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
        inject.close();
        data.append(bytes);
        return(data.length());
    }
    return(-1);
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

    if(m_settings.enabled) {
        QWebElement checker = body.findFirst("#clickChecker");
        if(!checker.isNull()) checker.setAttribute("checked", "checked");
    }

    di.truncate(0);
    if(readDataFile("locations.json", di) <= 0) {
        return;
    }
    QByteArray data;
    data.append(di);
    //QJsonDocument json = QJsonDocument::fromJson(data);
    //qDebug() << json.toJson();

    di.truncate(0);
    if(readDataFile("checkscript.js", di) <= 0) {
        return;
    }
    di.prepend("\nvar km_locations = " + data + ";\n");
    mainFrame->evaluateJavaScript(di);
}
