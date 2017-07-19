#ifndef BKACCOUNTMODULE_H
#define BKACCOUNTMODULE_H

#include "bkmodule.h"



class bkAccountModule : public bkModule
{
public:
    bkAccountModule(QJsonObject& defaults);

public slots:
    void parse(QWebPage*);
    void logic(QWebPage*);
    void view(QWebPage*);

private slots:
    void checkAccount(QWebPage*);
};

#endif // BKACCOUNTMODULE_H
