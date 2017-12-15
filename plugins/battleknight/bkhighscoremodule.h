#ifndef BKHIGHSCOREMODULE_H
#define BKHIGHSCOREMODULE_H
#include "bkmodule.h"




class bkHighscoreModule : public bkModule
{
    Q_OBJECT
public:
    bkHighscoreModule(QJsonObject& defaults, bkAccount* account, QObject *parent = 0);

public slots:
    void restoreState(QJsonObject& state);
    void parse(QWebPage* page, QVariant titleTimer = QVariant(), QVariant progressbarEndTime = QVariant(), QVariant progressbarDuration = QVariant());
    void logic(QWebPage*);
    void action(QWebPage*);
    void view(QWebPage*);

protected slots:
    void readKnightRow(QWebElement row);
};

#endif // BKHIGHSCOREMODULE_H
