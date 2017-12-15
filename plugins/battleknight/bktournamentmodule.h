#ifndef BKTOURNAMENTMODULE_H
#define BKTOURNAMENTMODULE_H
#include "bkmodule.h"

#include <QDateTime>




class bkTournamentModule : public bkModule
{
    Q_OBJECT
public:
    bkTournamentModule(QJsonObject& defaults, bkAccount* account, QObject *parent = 0);

    enum modes {
        unknown,
        no_login = 601,
        do_login,
        is_login,
        prepare,
        round
    };

public slots:
    void parse(QWebPage* page, QVariant titleTimer = QVariant(), QVariant progressbarEndTime = QVariant(), QVariant progressbarDuration = QVariant());
    void logic(QWebPage*);
    void action(QWebPage*);
    void view(QWebPage*);

protected slots:
    void joustHtml(QWebPage*, QVariant progressbarEndTime = QVariant(), QVariant progressbarDuration = QVariant());

private:
    QDateTime                           m_lastUpdate;
};

#endif // BKTOURNAMENTMODULE_H
