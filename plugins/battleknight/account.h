#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QObject>
#include <QWebPage>
#include <QVariant>

#include <QJsonDocument>
#include <QJsonObject>

#include "playermanager.h"
#include "itemmanager.h"
#include "reportmanager.h"

class botConfig
{
public:
    bool bot;
};

class accPlayer
{
public:
    int     id;
    QString name;
    QString rang;
    int     level;
    int     experience;
    int     life;
    int     maxLife;
    int     silver;
    int     treasury;
    int     rubies;
    QString course;
    int     karma;

    bool    premium;
};

/*
 * Orte in BattleKnight (locations.json)
 * id   location            name
 * _________________________________
 *
 * v1   VillageOne          Tarant
 * tp1  TradingPostOne      Grand
 * h3   HarbourThree        Sedwich
 * hs   CapitalCity         Endalain
 * cf1  CoastalFortressOne  Asgal
 * cf2  CoastalFortressTwo  Gastain
 * c1   CityOne             Alcran
 * v2   VillageTwo          Hatwig
 * tp2  TradingPostTwo      Talmet
 * h1   HarbourOne          Waile
 * h2   HarbourTwo          Alvan
 * tp4  TradingPostFour     Milley
 * v4   VillageFour         Jarow
 * f2   FortressTwo         Segur
 * tp3  TradingPostThree    Brant
 * v3   VillageThree        Ramstill
 * f1   FortressOne         Thulgar
 * gt   GhostTown           Talfour

    <div id="h1tp3" class="path"></div>
    <div id="tp2h1" class="path"></div>
    <div id="hstp2" class="path"></div>
    <div id="tp1hs" class="path"></div>
    <div id="v1tp1" class="path"></div>
    <div id="tp1gt" class="path"></div>
    <div id="h3tp1" class="path"></div>
    <div id="hscf1" class="path"></div>
    <div id="gtc1" class="path"></div>
    <div id="c1cf2" class="path"></div>
    <div id="tp2v2" class="path"></div>
    <div id="v3tp3" class="path"></div>
    <div id="tp3f1" class="path"></div>
    <div id="c1tp3_longboat" class="path"></div>
*/

class QTimerEvent;
class PlayerManager;
class ItemManager;
class ReportManager;

class Account : public QObject
{
    Q_OBJECT

public:
    Account(const QString cookie, const QUrl url, QObject *parent = 0);

    Q_INVOKABLE bool isActive(const QString option = "enableAccount") const {
        if(m_accStatus.contains(option)) return(m_accStatus.value(option).toBool());
        return(false);
    }
    Q_INVOKABLE QString cookieValue() const { return(m_cookieValue); }
    Q_INVOKABLE void setProfile(const QVariant data);
    Q_INVOKABLE void setStatus(const QVariant data);
    Q_INVOKABLE QString profile(const QString key) const;
    Q_INVOKABLE QString player(const QString id, const QString key) const;
    Q_INVOKABLE QString reports(const int count, const QString type) const;
    Q_INVOKABLE QVariant status(const QString key) const;

    void loadFinished(QWebPage*);
    void replyFinished(QNetworkReply*);

    QString fingerprint() const;
    QJsonObject state();
    void restoreState(const QJsonObject&);

protected:
    void timerEvent(QTimerEvent *event);

public slots:
    void toggle(const QString option = "enableAccount", const bool soll = false);
    void setPlayer(const QVariant data) {
        s_playerManager->checkPlayer(data);
    }
    void setItem(const QVariant data) {
        s_itemManager->checkItem(data);
    }

private slots:
    void reasonCleaner();

signals:
    void playerFound();

private:
    QString                 m_cookieValue;
    QNetworkAccessManager*  s_networkManager;
    int                     m_accPlayer;
    QString                 m_accHost;

    int                     m_cleanTimer;
    int                     m_missionsTimer;
    int                     m_gmTimer;

    PlayerManager*          s_playerManager;
    ItemManager*            s_itemManager;
    ReportManager*          s_reportManager;

    QVariantMap             m_player;
    QJsonObject             m_accStatus;
};

#endif // ACCOUNT_H
