#include "itemmanager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QDebug>

ItemManager::ItemManager(QObject *parent) :
    QObject(parent)
{
}

ItemManager::~ItemManager()
{
}

void ItemManager::checkItem(const QVariant data)
{
    QJsonDocument json = QJsonDocument::fromVariant(data);
    if(!json.isObject()) return;

    // die leidigen Zettel...
    QJsonObject item = json.object();
    if(item.contains("clue_item") && item.value("clue_item").toString() != "") {
        //if(item.value("clue_status").toString() == "Mission") qDebug() << json.toJson();
    }
}
