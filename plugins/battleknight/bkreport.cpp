#include "bkreport.h"

#include <QJsonDocument>
#include <QJsonValue>
#include <QDebug>



bkReport::bkReport(int id, QObject *parent) :
    QObject(parent),
    report_id(id)
{
    //qDebug() << "bkReport [" << report_id << "] created.";

    // "" = Ordensschlacht
    //("war_attacker", "war_castle", "war_defender", "war_endtime", "war_fight_data", "war_id", "war_initiator", "war_looted_silver", "war_payment_data", "war_result", "war_rounds", "war_starttime", "war_type")
    // fight_type("warfight") = Kriegsduell
    // fight_type("duel") = Duell
    // fight_type("mob") = Mission
    // fight_type("group") = Gruppenmission
    // fight_type("joust") = Turnier
    //
    // report_id = fight_id oder war_id
    m_keys << "aggressor" << "aggressor_id" << "defender" << "defender_id"
           << "fight_id" << "fight_log" << "fight_stats" << "fight_time" << "fight_type";
}

bkReport::~bkReport()
{
    //qDebug() << "bkReport [" << report_id << "] destroyed.";
}

void bkReport::setData(const QJsonObject& data)
{

    foreach(QString key, data.keys()) {
        if(m_keys.contains(key))
            m_data.insert(key,data.value(key));
    }

    //QJsonDocument debug(m_data); qDebug() << "\t" << "bkReport::setData" << report_id << debug.toJson().data();
}
