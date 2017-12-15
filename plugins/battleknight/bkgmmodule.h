#ifndef BKGMMODULE_H
#define BKGMMODULE_H

#include "bkmodule.h"
#include <QTimer>




class bkGMModule : public bkModule
{
    Q_OBJECT
public:
    bkGMModule(QJsonObject& defaults, bkAccount* account, QObject *parent = 0);

    enum modes {
        unknown,
        waiting,
        no_points,
        no_group,
        group_found,
        wearing,
        join_group,
        in_group
    };

public slots:
    void replyFinished(QNetworkReply*);
    void parse(QWebPage*, QVariant titleTimer = QVariant(), QVariant progressbarEndTime = QVariant(), QVariant progressbarDuration = QVariant());
    void logic(QWebPage*);
    void action(QWebPage*);
    void view(QWebPage*);

protected slots:
    void adjustTimer();
    void adjustPoints();
    void getGroups(QWebElement);
    void groupInfo(QWebPage*);

    void actionTimer();
    void clickTavern();

private:
    QTimer                      mTimer;
};

#endif // BKGMMODULE_H
