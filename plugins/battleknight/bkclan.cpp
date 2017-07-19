#include "bkclan.h"

#include <QJsonValue>

#include <QDebug>



bkClan::bkClan(int id, QObject *parent) :
    QObject(parent),
    clan_id(id)
{
    //qDebug() << "bkClan [" << clan_id << "] created.";
    m_keys << "clan_id" << "clan_tag" << "clan_name" << "clan_castle";
}

bkClan::~bkClan()
{
    //qDebug() << "bkClan [" << clan_id << "] destroyed.";
}

void bkClan::setData(const QJsonObject& data)
{

    foreach(QString key, data.keys()) {
        if(m_keys.contains(key))
            m_data.insert(key,data.value(key));
    }

    //QJsonDocument debug(m_data);    qDebug() << "\t" << "bkClan::setData" << clan_id << debug.toJson().data();
}
