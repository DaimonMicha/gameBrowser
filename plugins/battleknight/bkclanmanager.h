#ifndef BKCLANMANAGER_H
#define BKCLANMANAGER_H

#include <QObject>
#include <QMap>

#include "bkclan.h"




class bkClanManager : public QObject
{
    Q_OBJECT
public:
    explicit bkClanManager(QObject *parent = 0);
    ~bkClanManager();

    bkClan* clan(const int id);

signals:

public slots:

private:
    QMap<int, bkClan*>        m_clanMap;
};

#endif // BKCLANMANAGER_H
