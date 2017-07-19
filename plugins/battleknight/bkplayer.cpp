#include "bkplayer.h"

#include <QDebug>




bkPlayer::bkPlayer(int player_id, QObject *parent) :
    QObject(parent),
    knight_id(player_id)
{
    //qDebug() << "bkPlayer [" << knight_id << "] created.";
    m_keys << "knight_id" << "knight_rang" << "knight_name"
           << "knight_level" << "strength" << "dexterity"
           << "endurance" << "luck" << "weapon"
           << "shield" << "damage_min" << "damage_max"
           << "armour" << "offensive" << "defensive"
           << "clan_id" << "clan_rang" << "silver_spend"
           << "rubies_spend" << "turniere_won" << "loot_won"
           << "loot_lose" << "fights" << "fights_won"
           << "fights_lose" << "fights_balance" << "knight_course"
           << "karma" << "gender" << "silver"
           << "karma_active"
           << "rubies" << "experience" << "hitZones"
           << "defendZones" << "turnierHitZones" << "turnierDefendZones"
           << "helmet" << "jewellery" << "companion" << "horse"
           << "chest" << "shield" << "lance" << "weapon"
           << "manor_royalty" << "manor_guards" << "manor_trainer"
           << "manor_treasurer" << "manor_comrade" << "manor_healer"
           << "lastDuel" << "lastUpdated";
}

bkPlayer::~bkPlayer()
{
    //qDebug() << "bkPlayer [" << knight_id << "] destroyed.";
}

bool bkPlayer::isGreaterThan(bkPlayer* other)
{
    bool ret = false;
    if(m_data.value("knight_level").isUndefined()) {
        qDebug() << "bkPlayer::isGreaterThan(knight_level) is undefined.";
        return ret;
    }
    signed int level = other->value("knight_level").toInt() - m_data.value("knight_level").toInt();
    if(level < -1) { // reports checken
        ret = true;
    }
    if(other->value("clan_id").isValid()) ret = false;
    //qDebug() << "bkPlayer::isGreaterThan" << other->value("clan_id");

    return ret;
}

QVariant bkPlayer::value(const QString& key)
{
    QVariant ret;

    if(m_keys.contains(key)) {
        ret = m_data.value(key).toVariant();
    }

    return ret;
}

void bkPlayer::setData(const QJsonObject& data)
{

    foreach(QString key, data.keys()) {
        if(m_keys.contains(key))
            m_data.insert(key,data.value(key));
    }
    //QJsonDocument debug(m_data);    qDebug() << "bkPlayer::setData" << knight_id << debug.toJson().data();
}

QDebug operator << (QDebug dbg, const bkPlayer* player)
{
    dbg.space() << player->toJson().data();
    return dbg.maybeSpace();
}
