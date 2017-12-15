#include "bkworld.h"
#include "bkplayer.h"
#include "bkclan.h"
#include "bkitem.h"

#include <QJsonDocument>

#include <QDebug>





bkWorld::bkWorld(const QString& name, QObject *parent) :
    QObject(parent),
    m_name(name),
    p_playerManager(new bkPlayerManager(this)),
    p_clanManager(new bkClanManager(this)),
    p_itemManager(new bkItemManager(this)),
    p_reportManager(new bkReportManager(this))
{
    //qDebug() << "bkWorld" << m_name << "created.";
}

bkWorld::~bkWorld()
{
    delete p_playerManager;
    delete p_clanManager;
    delete p_itemManager;
    delete p_reportManager;
    qDebug() << "bkWorld" << m_name << "destroyed.";
}

QJsonObject bkWorld::locationId(const QString& id)
{
    foreach(QString sh, m_locations.keys()) {
        if(sh == id) return m_locations.value(id).toObject();
    }
    return QJsonObject();
}

QJsonObject bkWorld::locationName(const QString& name)
{
    foreach(QJsonValue loc, m_locations) {
        if(name == loc.toObject().value("name").toString()) return loc.toObject();
    }
    return QJsonObject();
}

QJsonObject bkWorld::karmaId(const QString& id)
{
    foreach(QString sh, m_karma.keys()) {
        if(sh == id) return m_karma.value(id).toObject();
    }
    return QJsonObject();
}

void bkWorld::setLocationStrings(const QString& loc)
{
    int pos = loc.indexOf("/*");
    QString locations(loc);
    if(pos) locations = loc.left(pos);
    QJsonDocument tmp = QJsonDocument::fromJson(locations.toLocal8Bit());
    if(tmp.isObject()) {
        m_locations = tmp.object();
        //qDebug() << "bkWorld::setLocationStrings" << tmp.toJson().data();
    }
    //else qDebug() << locations.toLocal8Bit().data();
}

void bkWorld::setKarmaStrings(const QString& karma)
{
    QJsonDocument tmp = QJsonDocument::fromJson(karma.toLocal8Bit());
    if(tmp.isObject()) {
        m_karma = tmp.object();
        //qDebug() << "bkWorld::setKarmaStrings" << tmp.toJson().data();
    }
    //else qDebug() << karma.toLocal8Bit().data();
}
