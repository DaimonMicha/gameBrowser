#ifndef BKTREASURYMODULE_H
#define BKTREASURYMODULE_H
#include "bkmodule.h"




class bkTreasuryModule : public bkModule
{
    Q_OBJECT
public:
    bkTreasuryModule(QJsonObject& defaults, bkAccount* account, QObject *parent = 0);

public slots:
    void parse(QWebPage* page, QVariant titleTimer = QVariant(), QVariant progressbarEndTime = QVariant(), QVariant progressbarDuration = QVariant());
    void logic(QWebPage*);
    void action(QWebPage*);
    void view(QWebPage*);
};

#endif // BKTREASURYMODULE_H
