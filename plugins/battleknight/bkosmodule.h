#ifndef BKOSMODULE_H
#define BKOSMODULE_H
#include "bkmodule.h"




class bkOSModule : public bkModule
{
    Q_OBJECT
public:
    bkOSModule(QJsonObject& defaults, bkAccount* account, QObject *parent = 0);

public slots:
    void parse(QWebPage* page, QVariant titleTimer = QVariant(), QVariant progressbarEndTime = QVariant(), QVariant progressbarDuration = QVariant());
    void logic(QWebPage*);
    void action(QWebPage*);
    void view(QWebPage*);

private slots:
    void tavernPrepareHtml(QWebPage* page, QVariant progressbarEndTime, QVariant progressbarDuration);
    void tavernBattleHtml(QWebPage* page, QVariant progressbarEndTime, QVariant progressbarDuration);
};

#endif // BKOSMODULE_H
