#ifndef BKTRAVELMODULE_H
#define BKTRAVELMODULE_H
#include "bkmodule.h"




class bkTravelModule : public bkModule
{
    Q_OBJECT
public:
    bkTravelModule(QJsonObject& defaults, bkAccount* account, QObject *parent = 0);

    enum modes {
        travel_to = 1000
    };

public slots:
    void restoreState(QJsonObject& state);
    void replyFinished(QNetworkReply*);
    void parse(QWebPage* page, QVariant titleTimer = QVariant(), QVariant progressbarEndTime = QVariant(), QVariant progressbarDuration = QVariant());
    void logic(QWebPage*);
    void action(QWebPage*);
    void view(QWebPage*) {}

protected slots:
    void reply_startTravel(QNetworkReply*);

    void parse_worldLocation(QWebPage*);
    void checkEvents(QWebElement document);
    void parse_worldMap(QWebPage*);
    void parse_worldTravel(QWebPage*);
};

#endif // BKTRAVELMODULE_H
