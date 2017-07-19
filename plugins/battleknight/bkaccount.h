#ifndef BKACCOUNT_H
#define BKACCOUNT_H

#include <QObject>
#include <QPointer>
#include <QJsonObject>
#include <QWebFrame>
#include <QTimer>

#include "bkmodulemanager.h"
#include "bkparser.h"
#include "bklogic.h"
#include "bkworld.h"
#include "bkplayer.h"

#include <QDebug>

class QWebPage;
class QNetworkReply;
//class accModule;
class bkModuleManager;

enum cooldownModes {
    cd_unknown,
    cd_none,
    cd_work,
    cd_duel,
    cd_fight,
    cd_travel
};

enum clanwarModes {
    cw_unknown,
    cw_no_war,
    cw_prepare,
    cw_round
};

enum gmModes {
    gm_unknown,
    gm_no_group_found,
    gm_group_found,
    gm_in_group,
    gm_no_points
};

/*
 * class bkAccount
 */
class bkAccount : public QObject
{
    Q_OBJECT
public:
    explicit bkAccount(bkWorld* world, int player_id, QJsonObject defaults, QObject *parent = 0);
    ~bkAccount();

    QJsonObject saveState();
    void restoreState(QJsonObject);

    QWebPage* currentPage();
    bkWorld* world() const { return p_world; }
    bkParser* parser() const { return p_parser; }
    bkPlayer* player(const int id = 0) const {
        if(id == 0) return p_knight;
        else return p_world->player(id);
    }

    Q_INVOKABLE QVariant kmPlayer(const int id = 0) {
        QVariant ret;
        bkPlayer* p = player(id);
        ret = QJsonDocument::fromJson(p->toJson()).toVariant();
        return ret;
    }
    Q_INVOKABLE QVariant module(const QString& key = "Account");
    Q_INVOKABLE bool enabled(const QString& key = "Account");
    Q_INVOKABLE QVariant status(const QString key);
    Q_INVOKABLE QVariant logicStrength(const QString& topic);

    friend QDebug operator << (QDebug dbg, const bkAccount* account);

private:
    void adjustMissionTimer();
    void adjustGMTimer();

signals:

public slots:
    void toggle(const QString option = "enableAccount", const bool soll = false);
    void setStatus(const QString&, const QString&, QVariant&);
    void setStatus(const QString&, QVariant&);
    void replyFinished(QNetworkReply*);
    void loadFinished(QWebPage*);
    void loadUrl(const QUrl& url);

private slots:
    void missionTimer();
    void gmTimer();

private:
    //QJsonObject                 m_config;
    QJsonObject                 m_status;
    bkWorld*                    p_world;
    bkParser*                   p_parser;
    bkLogic*                    p_logic;
    bkPlayer*                   p_knight;
    bkModuleManager*            p_moduleManager;

    QTimer                      t_mpTimer;
    QTimer                      t_gmTimer;

    QList<QPointer<QWebPage> >  m_webPages;
    QPointer<QWebPage>          m_currentPage;
};

#endif // BKACCOUNT_H
