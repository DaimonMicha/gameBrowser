#include "bkplayermanager.h"

#include <QDebug>




bkPlayerManager::bkPlayerManager(QObject *parent) :
    QObject(parent)
{
    qDebug() << "bkPlayerManager created.";
}

bkPlayerManager::~bkPlayerManager()
{
    int pc = m_playerMap.count();
    while(!m_playerMap.isEmpty()) {
        bkPlayer* player = m_playerMap.take(m_playerMap.lastKey());
        delete player;
    }
    qDebug() << "bkPlayerManager and" << pc << "Player(s) destroyed.";
}

bkPlayer* bkPlayerManager::player(const int id)
{
    if(m_playerMap.contains(id)) {
        return m_playerMap.value(id);
    }
    bkPlayer* player = new bkPlayer(id);
    m_playerMap.insert(id, player);
    return player;
}
