#include "bkitemmanager.h"

#include <QDebug>



bkItemManager::bkItemManager(QObject *parent) :
    QObject(parent)
{
    qDebug() << "bkItemManager created.";
}

bkItemManager::~bkItemManager()
{
    int ic = m_itemMap.count();
    while(!m_itemMap.isEmpty()) {
        bkItem* item = m_itemMap.take(m_itemMap.lastKey());
        item->deleteLater();
    }
    qDebug() << "bkItemManager and" << ic << "Item(s) destroyed.";
}

bkItem* bkItemManager::item(const int id)
{
    if(m_itemMap.contains(id)) {
        return m_itemMap.value(id);
    }
    bkItem* item = new bkItem(id);
    m_itemMap.insert(id, item);
    return item;
}

QMap<int, bkItem*> bkItemManager::itemsFor(const int knight_id)
{
    QMap<int, bkItem*> ret;

    foreach(bkItem* item, m_itemMap) {
        if(item->value("item_own").toInt() == knight_id) {
            ret.insert(item->id(), item);
        }
    }

    return ret;
}
