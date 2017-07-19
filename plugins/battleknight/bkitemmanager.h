#ifndef BKITEMMANAGER_H
#define BKITEMMANAGER_H

#include <QObject>
#include <QMap>

#include "bkitem.h"



class bkItemManager : public QObject
{
    Q_OBJECT
public:
    explicit bkItemManager(QObject *parent = 0);
    ~bkItemManager();

    bkItem* item(const int id);
    QMap<int, bkItem*> itemsFor(const int knight_id);

signals:

public slots:

private:
    QMap<int, bkItem*>        m_itemMap;
};

#endif // BKITEMMANAGER_H
