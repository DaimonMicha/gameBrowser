#ifndef BKPLAYERMANAGER_H
#define BKPLAYERMANAGER_H

#include <QObject>
#include <QMap>

#include "bkplayer.h"



class bkPlayerManager : public QObject
{
    Q_OBJECT
public:
    explicit bkPlayerManager(QObject *parent = 0);
    ~bkPlayerManager();

    bkPlayer* player(const int id);

signals:

public slots:

private:
    QMap<int, bkPlayer*>        m_playerMap;
};

#endif // BKPLAYERMANAGER_H
