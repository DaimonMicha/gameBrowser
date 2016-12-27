#include "playermanager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QStandardItemModel>

#include <QDebug>




PlayerManager::PlayerManager(QObject *parent) :
    QObject(parent),
    s_playerModel(new QStandardItemModel)
{
    head << "knight_id" << "knight_rang" << "knight_name" << "knight_level"
         << "strength" << "dexterity" << "endurance" << "luck" << "weapon" << "shield"
         << "damage_min" << "damage_max" << "armour" << "offensive" << "defensive"
         << "clan_id" << "clan_rang" << "silver_spend" << "rubies_spend" << "turniere"
         << "loot_won" << "loot_lose" << "fights_won" << "fights_lose" << "fights_balance"
         << "knight_course" << "karma" << "gender" << "silver" << "rubies" << "experience"
         << "hitZones" << "defendZones" << "lastFight";
    s_playerModel->setHorizontalHeaderLabels(head);
}

void PlayerManager::checkPlayer(const QVariant data)
{
    QJsonDocument json = QJsonDocument::fromVariant(data);
    if(!json.isObject()) return;

    //qDebug() << json.toJson();
}
