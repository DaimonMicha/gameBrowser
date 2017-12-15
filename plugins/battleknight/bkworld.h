#ifndef BKWORLD_H
#define BKWORLD_H

#include <QObject>
#include <QJsonObject>

#include "playermanager.h"
#include "bkplayermanager.h"
#include "bkclanmanager.h"
#include "bkitemmanager.h"
#include "bkreportmanager.h"


class bkPlayer;
class bkClan;
class bkItem;
class bkReport;

/*
 * class bkWorld
 *
 * holds an player-manager, an clan-manager, an item-manager
 * and an report-manager.
 */
class bkWorld : public QObject
{
    Q_OBJECT
public:
    explicit bkWorld(const QString& name, QObject *parent = 0);
    ~bkWorld();

    const QString& name() const { return m_name; }

    bkPlayer* player(int id) {
        return p_playerManager->player(id);
    }
    QMap<int, bkItem*> itemsFor(const int knight_id) {
        return p_itemManager->itemsFor(knight_id);
    }
    bkItem* item(int id) {
        return p_itemManager->item(id);
    }
    bkClan* clan(int id) {
        return p_clanManager->clan(id);
    }
    bkReport* report(int id) {
        return p_reportManager->report(id);
    }

    QJsonObject locationId(const QString&);
    QJsonObject locationName(const QString&);

    QJsonObject karmaId(const QString&);


signals:

public slots:
    void setLocationStrings(const QString&);
    void setKarmaStrings(const QString&);

private:
    QString             m_name;
    bkPlayerManager*    p_playerManager;
    bkClanManager*      p_clanManager;
    bkItemManager*      p_itemManager;
    bkReportManager*    p_reportManager;

    QJsonObject         m_locations;
    QJsonObject         m_karma;
};

#endif // BKWORLD_H
