#include "account.h"

#include <QUrl>
#include <QDateTime>
#include <QWebFrame>
#include <QNetworkReply>

#include <QDebug>


Account::Account(const QString cookie, QObject *parent) :
    QObject(parent),
    m_cookieValue(cookie),
    s_networkManager(0)
{
    m_config.bot = false;
}

void Account::toggle(const QString option, const bool on)
{
    if(option == "account") {
        m_config.bot = on;
        if(on) {
        } else {
        }
    }
    qDebug() << "Account::toggle:" << option << on;
}

void Account::loadFinished(QWebPage* page)
{
    QWebFrame* mainFrame = page->mainFrame();
    QUrl url = mainFrame->url();
    QStringList paths = url.path().split("/",QString::SkipEmptyParts);

    if(!paths.count()) return; // nothing to do, login evtl?

    if(!s_networkManager) s_networkManager = page->networkAccessManager();
}

void Account::replyFinished(QNetworkReply* reply)
{
    QUrl url = reply->url();
    QStringList paths = url.path().split("/",QString::SkipEmptyParts);

    if(!paths.count()) return; // nothing to do, login evtl?

    if(!s_networkManager) s_networkManager = reply->manager();
}
