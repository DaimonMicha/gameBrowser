#ifndef BKPARSER_H
#define BKPARSER_H

#include <QObject>
#include <QJsonObject>
#include <QWebPage>
#include <QWebElement>



class bkAccount;
class QNetworkReply;
class QNetworkAccessManager;

class bkParser : public QObject
{
    Q_OBJECT
public:
    explicit bkParser(QObject *parent = 0);
    ~bkParser();

    // Htmls
    void joustHtml(QWebPage*);
    void treasuryHtml(QWebPage*);
    void userHtml(QWebPage*);
    void commonProfileHtml(QWebPage*);
    void userKarmaHtml(QWebPage*);
    void groupmissionGroupHtml(QWebPage*);
    void worldHtml(QWebPage*);
    void duelHtml(QWebPage*);
    void duelCompareHtml(QWebPage*);
    void duelDuelHtml(QWebPage*);
    void highscoreHtml(QWebPage*);
    void tavernPrepareHtml(QWebPage*);
    void tavernBattleHtml(QWebPage*);
    // Reports
    void mailInbox(const QByteArray& data);
    // Items
    void ajaxGetInventory(const QByteArray& data);
    void ajaxBuyItem(const QByteArray& data);
    // Knights
    void ajaxProposal(const QByteArray& data);

private:
    QJsonDocument unserialize(const QString&);
    void translatePlayer(const QJsonObject&);

signals:

public slots:
    void replyFinished(QNetworkReply*);
    void loadFinished(QWebPage*);
    void callReports();

private slots:
    void checkAccount(QWebPage*);
    void checkReplyPaths(QNetworkReply*);

private:
    bkAccount*                  p_account;
    QWebPage*                   p_page;
    QNetworkAccessManager*      p_networkManager;
};

#endif // BKPARSER_H
