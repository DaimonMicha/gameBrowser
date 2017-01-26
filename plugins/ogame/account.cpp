#include "account.h"
#include "ogame.h"

#include <QUrl>
#include <QDateTime>
#include <QWebFrame>
#include <QNetworkReply>
#include <QUrlQuery>

#include <QDebug>


Account::Account(const QString cookie, QObject *parent) :
    QObject(parent),
    m_cookieValue(cookie),
    s_networkManager(0)
{
}

void Account::toggle(const QString option, const bool soll)
{
    bool ist = isActive(option);

    if(option == "enableAccount") {
        if(soll) {
        } else {
        }
    }

    if(ist != soll) {
        m_botOptions.insert(option, soll);
    }

    qDebug() << parent()->metaObject()->className() << "Account::toggle:" << option << "von" << ist << "zu" << soll;
}

void Account::loadFinished(QWebPage* page)
{
    if(!s_networkManager) s_networkManager = page->networkAccessManager();
    QWebFrame* mainFrame = page->mainFrame();
    QUrl url = mainFrame->url();
    QStringList paths = url.path().split("/",QString::SkipEmptyParts);

    OGame* plugin = qobject_cast<OGame *>(parent());
    if(plugin) {
        QString script;
        plugin->readDataFile("checkscript.js",script);
        mainFrame->evaluateJavaScript(script);
    } else {
        return;
    }

    if(!paths.count()) return; // nothing to do, login evtl?

    QUrlQuery q(url.query());
    if(!q.hasQueryItem("page")) return;
    QString pg = q.queryItemValue("page");

}

void Account::replyFinished(QNetworkReply* reply)
{
    if(!s_networkManager) s_networkManager = reply->manager();
    QUrl url = reply->url();
    QStringList paths = url.path().split("/",QString::SkipEmptyParts);

    if(!paths.count()) return; // nothing to do

    if(paths.at(0) == QString("socket.io")) {
        qDebug() << "\t" << reply->property("getData").toByteArray();
    }

}
