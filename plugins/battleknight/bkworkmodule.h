#ifndef BKWORKMODULE_H
#define BKWORKMODULE_H
#include "bkmodule.h"




class bkWorkModule : public bkModule
{
    Q_OBJECT
public:
    bkWorkModule(QJsonObject& defaults, bkAccount* account, QObject *parent = 0);

    enum modes {
        unknown,
        waiting,
        go_market,
        travel,
        go_working,
        working,
        encash
    };

public slots:
    void replyFinished(QNetworkReply*);
    void parse(QWebPage* page, QVariant titleTimer = QVariant(), QVariant progressbarEndTime = QVariant(), QVariant progressbarDuration = QVariant());
    void logic(QWebPage*);
    void action(QWebPage*);
    void view(QWebPage*);

protected slots:
    void actionTimer();
    void clickMarket();
    void goWorking();
    void getCash();
};

#endif // BKWORKMODULE_H
