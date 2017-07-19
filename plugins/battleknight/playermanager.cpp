#include "playermanager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QStandardItemModel>
#include <QDateTime>

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
         << "hitZones" << "defendZones" << "lastDuel" << "lastSeen";
    s_playerModel->setHorizontalHeaderLabels(head);
}

void PlayerManager::checkPlayer(const QVariant data)
{
    QJsonDocument json = QJsonDocument::fromVariant(data);
    if(!json.isObject()) return;

    QJsonObject player = json.object();
    if(!player.contains("knight_id")) return;

    int pid = player.value("knight_id").toInt();
    int row = playersRow(pid);
    if(row < 0) row = createRow(pid);

    QString dateFormat = "yyyy-MM-dd hh:mm:ss";
    QDateTime now;
    QDateTime saved = QDateTime::fromString(s_playerModel->item(row, head.indexOf("lastSeen"))->data(Qt::DisplayRole).toString(), dateFormat);
    if(player.contains("report_time")) {
        now = QDateTime::fromString(player.value("report_time").toString(), dateFormat);
        QDateTime ld = QDateTime::fromString(s_playerModel->item(row, head.indexOf("lastDuel"))->data(Qt::DisplayRole).toString(), dateFormat);
        if(now > ld) s_playerModel->item(row, head.indexOf("lastSeen"))->setData(now.toString(dateFormat), Qt::DisplayRole);
    } else {
        now = QDateTime::currentDateTime();
    }
    // if(now < saved) return;
    foreach(QString key, head) {
        if(player.contains(key)) {
            s_playerModel->item(row, head.indexOf(key))->setData(player.value(key).toVariant(), Qt::DisplayRole);
        }
    }
    if(now > saved) {
        s_playerModel->item(row, head.indexOf("lastSeen"))->setData(now.toString(dateFormat), Qt::DisplayRole);

        //qDebug() << "\tPlayerManager::checkPlayer\n" << json.toJson();
    }
}

bkPlayer* PlayerManager::player(const int id)
{
    foreach(bkPlayer *p, m_playerList) {
        if(p->id() == id) {
            return p;
        }
    }
    bkPlayer* player = new bkPlayer(id);
    m_playerList.append(player);
    return player;
}

QVariant PlayerManager::playerData(const int id) const
{
    int row = playersRow(id);
    if(row < 0) return(QVariant());

    QVariantMap player;
    foreach(QString key, head) {
        player.insert(key, s_playerModel->item(row, head.indexOf(key))->data(Qt::DisplayRole));
    }

    return(QVariant(player));
}

int PlayerManager::playersRow(int id) const
{
    int ret = -1;
    QList<QStandardItem *> items = s_playerModel->findItems(QString("%1").arg(id), Qt::MatchExactly, head.indexOf("knight_id"));
    if(items.length()) {
        ret = items.at(0)->row();
    }
    return(ret);
}

int PlayerManager::createRow(int id) const
{
    QStandardItem *item;
    QList<QStandardItem *> items;
    for(int i = 0; i < head.length(); ++i) {
        if(i == head.indexOf("knight_id")) {
            item = new QStandardItem(QString("%1").arg(id));
            items.append(item);
        } else if(i == head.indexOf("lastSeen")) {
            item = new QStandardItem(QString("1970-01-01 00:00:00"));
            items.append(item);
        } else {
            items.append(new QStandardItem);
        }
    }
    s_playerModel->appendRow(items);
    return(item->row());
}
