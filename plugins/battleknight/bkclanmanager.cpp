#include "bkclanmanager.h"

#include <QDebug>



bkClanManager::bkClanManager(QObject *parent) :
    QObject(parent)
{
    //qDebug() << "bkClanManager created.";
}

bkClanManager::~bkClanManager()
{
    int cc = m_clanMap.count();
    while(!m_clanMap.isEmpty()) {
        bkClan* clan = m_clanMap.take(m_clanMap.lastKey());
        delete clan;
    }
    qDebug() << "bkClanManager and" << cc << "Clan(s) destroyed.";
}

bkClan* bkClanManager::clan(const int id)
{
    if(m_clanMap.contains(id)) {
        return m_clanMap.value(id);
    }
    bkClan* clan = new bkClan(id);
    m_clanMap.insert(id, clan);
    return clan;
}
