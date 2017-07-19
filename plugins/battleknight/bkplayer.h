#ifndef BKPLAYER_H
#define BKPLAYER_H

#include <QObject>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>

class bkPlayer : public QObject
{
    Q_OBJECT
public:
    explicit bkPlayer(int player_id, QObject *parent = 0);
    ~bkPlayer();

    int id() const { return knight_id; }
    QVariant value(const QString& key);
    QByteArray toJson() const {
        return QJsonDocument(m_data).toJson();
    }

    bool isGreaterThan(bkPlayer* other);

    friend QDebug operator << (QDebug dbg, const bkPlayer* player);

signals:

public slots:
    void setData(const QJsonObject& data);

private:
    int             knight_id;
    QStringList     m_keys;
    QJsonObject     m_data;

    QString     knight_rang;
    QString     knight_name;
    int         knight_level;
    int         strength;
    int         dexterity;
    int         endurance;
    int         luck;
    int         weapon;
    int         shield;
    int         damage_min;
    int         damage_max;
    int         armour;
    int         offensive;
    int         defensive;
    int         clan_id;
    QString     clan_rang;
    int         silver_spend;
    int         rubies_spend;
    int         turniere_won;
    int         loot_won;
    int         loot_lose;
    int         fights;
    int         fights_won;
    int         fights_lose;
    int         fights_balance;
    bool        knight_course; // true = good, false = evil
    int         karma;
    bool        gender; // true = male, false = female
    int         silver;
    int         rubies;
    int         experience;
    QList<int>  hitZones;
    QList<int>  defendZones;
    QList<int>  turnierHitZones;
    QList<int>  turnierDefendZones;
    QDateTime   lastDuel;
    QDateTime   lastUpdated;
};

#endif // BKPLAYER_H
