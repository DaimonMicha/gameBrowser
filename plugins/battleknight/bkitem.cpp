#include "bkitem.h"

#include <QJsonValue>
#include <QDebug>



bkItem::bkItem(int id, QObject *parent) :
    QObject(parent),
    item_id(id)
{
    //qDebug() << "bkItem [" << item_id << "] created.";
    m_keys << "item_id" << "clue_data" <<"clue_item"
           << "clue_location" << "clue_mission"
           << "clue_level" << "clue_status" << "item_add"
           << "item_armour" << "item_critical" << "item_damage"
           << "item_damage2" << "item_def" << "item_depth"
           << "item_desckey" << "item_dex" << "item_end"
           << "item_fire" << "item_fire_res" << "item_fullName"
           << "item_ice" << "item_ice_res" << "item_info"
           << "item_inventory" << "item_karma" << "item_level"
           << "item_luck" << "item_magic_level" << "item_manor_seconds"
           << "item_name" << "item_off" << "item_own"
           << "item_pic" << "item_poison" << "item_poison_res"
           << "item_ride" << "item_ruby" << "item_rune1"
           << "item_rune2" << "item_rune3" << "item_set"
           << "item_shield" << "item_shock" << "item_shock_res"
           << "item_slot" << "item_special_ability" << "item_speed"
           << "item_str" << "item_timestamp" << "item_use"
           << "item_value" << "item_weapon" << "item_width"
           << "quest_id";
}

bkItem::~bkItem()
{
    //qDebug() << "bkItem [" << item_id << "] destroyed.";
}

QVariant bkItem::value(const QString& key)
{
    QVariant ret;

    if(m_keys.contains(key)) {
        ret = m_data.value(key).toVariant();
    }

    return ret;
}

// buyItem:
// "clue_data":{"location":"FortressTwo","mission":"BurningVillage"}
void bkItem::setData(const QJsonObject& data)
{
    foreach(QString key, data.keys()) {
        if(key == "item_info") {
            //qDebug() << "\t" << key << data.value(key);
        }
        if(key == "clue_data" && data.value(key).isObject()) {
            QJsonObject clue = data.value(key).toObject();
            if(clue.contains("location")) {
                m_data.insert("clue_location",clue.value("location"));
            }
            if(clue.contains("mission")) {
                m_data.insert("clue_mission",clue.value("mission"));
            }
            //qDebug() << "\t" << key << data.value(key);
        }
        if(!m_keys.contains(key)) continue;
        m_data.insert(key,data.value(key));
    }
    if(data.contains("item_own") && data.value("item_own") != QJsonValue("28545")) {
        //"28545",
        qDebug() << "\t" << "bkItem::setData:\n" << item_id << this;
    }
}

QDebug operator << (QDebug dbg, const bkItem* item)
{
    dbg.space() << item->toJson().data();
    return dbg.maybeSpace();
}
