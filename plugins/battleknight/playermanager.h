#ifndef PLAYERMANAGER_H
#define PLAYERMANAGER_H

#include <QObject>
#include <QStringList>
#include "bkplayer.h"


class QStandardItemModel;


class PlayerManager : public QObject
{
    Q_OBJECT
public:
    explicit PlayerManager(QObject *parent = 0);

    bkPlayer* player(const int id);

    void checkPlayer(const QVariant data);
    QVariant playerData(const int id) const;

    int playersRow(int id) const;
    int createRow(int id) const;

signals:

public slots:

private:
    QStringList                 head;
    QStandardItemModel*         s_playerModel;
    QList<bkPlayer *>           m_playerList;
};

#endif // PLAYERMANAGER_H
