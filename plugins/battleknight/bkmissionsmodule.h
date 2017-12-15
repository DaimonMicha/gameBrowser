#ifndef BKMISSIONSMODULE_H
#define BKMISSIONSMODULE_H

#include "bkmodule.h"
#include <QTimer>




class bkMissionsModule : public bkModule
{
    Q_OBJECT
public:
    bkMissionsModule(QJsonObject& defaults, bkAccount* account, QObject *parent = 0);

    enum modes {
        unknown,
        no_points,
        get_reward
    };

public slots:
    void replyFinished(QNetworkReply*);
    void parse(QWebPage* page, QVariant titleTimer = QVariant(), QVariant progressbarEndTime = QVariant(), QVariant progressbarDuration = QVariant());
    void logic(QWebPage*);
    void action(QWebPage*);
    void view(QWebPage*);

    void restoreState(QJsonObject& state);

protected slots:
    void adjustTimer();
    void adjustPoints();

    void clickJourney();

private:
    QTimer                      mTimer;
};

#endif // BKMISSIONSMODULE_H
