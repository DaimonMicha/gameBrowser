#ifndef BKDUELSMODULE_H
#define BKDUELSMODULE_H

#include "bkmodule.h"




class bkDuelsModule : public bkModule
{
    Q_OBJECT
public:
    bkDuelsModule(QJsonObject& defaults, bkAccount* account, QObject *parent = 0);

    enum modes {
        unknown,
        vote,
        next_proposal,
        to_compare,
        compare,
        attack,
        result,
        cooldown,
        go_tournament
    };

public slots:
    void replyFinished(QNetworkReply*);
    void parse(QWebPage*, QVariant titleTimer = QVariant(), QVariant progressbarEndTime = QVariant(), QVariant progressbarDuration = QVariant());
    void logic(QWebPage*);
    void action(QWebPage*);
    void view(QWebPage*);

protected slots:
    void duelHtml(QWebPage*);
    void storePlayer(int pid, QWebElement playerRow);
    void duelCompareHtml(QWebPage*);
    void duelDuelHtml(QWebPage*);

    void actionTimer();
    void clickWeaker();
    void clickStronger();
    void clickCompare();
    void clickAttack();
    void clickTournament();
};

#endif // BKDUELSMODULE_H
