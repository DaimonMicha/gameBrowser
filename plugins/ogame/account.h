#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QObject>
#include <QWebPage>
#include <QVariant>
#include <QWebElement>
#include <QJsonObject>


struct PlayerInfo
{
    int         id;
    QString     name;
    QString     status;
    int         aid; // alliance
};

struct PlanetInfo
{
    int         id;
    int         pid;
    int         size;
    QString     name;
    QString     type;
    QString     coords;
    bool        hasMoon;
    int         moonId;
    int         moonSize;
    QString     moonName;
};

/*

https://s137-de.ogame.gameforge.com

/api/players.xml
/api/universe.xml
/api/highscore.xml?category=1&type=1
    0 Total
    1 Economy
    2 Research
    3 Military
    5 Military Built
    6 Military Destroyed
    4 Military Lost
    7 Honor
/api/alliances.xml
/api/serverData.xml
/api/playerData.xml
/api/localization.xml
/api/universes.xml

*/

class Account : public QObject
{
    Q_OBJECT

public:
    Account(const QString cookie, QObject *parent = 0);

    Q_INVOKABLE QString cookieValue() const { return(m_cookieValue); }
    Q_INVOKABLE bool isActive(const QString option = "enableAccount") const {
        if(m_botOptions.contains(option)) return(m_botOptions.value(option));
        return(false);
    }
    QJsonObject getPlanet(QString& pid);
    QJsonObject parsePlanets(QWebFrame*);
    QJsonObject getFleet(QString& fid);
    QJsonObject parseFleet(QWebElement&);
    int fleetCargo(QJsonObject&);
    int fleetCapacity(QJsonObject&);

signals:

public slots:
    void toggle(const QString option = "account", const bool on = false);
    void loadFinished(QWebPage*);
    void replyFinished(QNetworkReply*);

private:
    QMap<QString, bool>     m_botOptions;
    QString                 m_cookieValue;
    QNetworkAccessManager*  s_networkManager;
    QMap<int, PlanetInfo*>  m_myPlanets;

    QString                 m_currentPlayer;
    QString                 m_currentPlanet;
    QJsonObject             m_planets;
    QJsonObject             m_fleets;
    QJsonObject             m_constants;
};

#endif // ACCOUNT_H
