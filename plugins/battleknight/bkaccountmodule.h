#ifndef BKACCOUNTMODULE_H
#define BKACCOUNTMODULE_H

#include "bkmodule.h"



class bkAccountModule : public bkModule
{
    Q_OBJECT
public:
    bkAccountModule(QJsonObject& defaults, bkAccount* account, QObject *parent = 0);

public slots:
    void replyFinished(QNetworkReply*);
    void parse(QWebPage*, QVariant titleTimer = QVariant(), QVariant progressbarEndTime = QVariant(), QVariant progressbarDuration = QVariant());
    void logic(QWebPage*);
    void action(QWebPage*);
    void view(QWebPage*);

    void restoreState(QJsonObject& state);

protected slots:
    void job(const QString& jobName, const int& mode);

    void getInventory(QNetworkReply*);
    void buyItem(QNetworkReply*);
    void sellItem(QNetworkReply*);
    void placeItem(QNetworkReply*);
    void wearItem(QNetworkReply*);
    void startTravel(QNetworkReply*);

    void checkAccount(QWebPage*);
    void profileHtml(QWebPage*);
    void userHtml(QWebPage*);
    void userKarmaHtml(QWebPage*, QVariant progressbarEndTime = QVariant(), QVariant progressbarDuration = QVariant());
    void worldTravelHtml(QWebPage*);
    void worldMapHtml(QWebPage*);
    void manorHtml(QWebPage*);
};

#endif // BKACCOUNTMODULE_H
