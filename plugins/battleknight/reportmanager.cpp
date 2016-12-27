#include "reportmanager.h"
#include "account.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QProcess>

#include <QDebug>




ReportManager::ReportManager(QObject *parent) :
    QObject(parent)
{
    phpProc = new QProcess();
    connect(phpProc, SIGNAL(readyReadStandardOutput()),
            this, SLOT(phpReady()));
}

void ReportManager::checkReport(const QVariant data)
{
    QJsonDocument json = QJsonDocument::fromVariant(data);
    if(!json.isObject()) return;

    if(m_mailsMap.contains(json.object().value("mail_id").toString())) {
        //if(json.object().value("mail_id").toString() == "60030088") qDebug() << m_mailsMap.value(json.object().value("mail_id").toString()).toMap();
        return;
    }

    QJsonObject content = json.object().value("mail_content").toObject();

    QJsonDocument debug(content);

    m_currentFightTime = content.value("fight_time").toString();

    qDebug() << "\nReportManager::checkReport" << json.object().value("mail_id")
                << content.value("fight_time").toString()
                << content.value("fight_type").toString()
                << content.value("fight_id").toString();
    unserialize(content.value("aggressor").toString().replace("O:13:\"FighterObject\":","a:"));
    unserialize(content.value("defender").toString().replace("O:13:\"FighterObject\":","a:"));
    //unserialize(content.value("fight_log").toString());
    // fight_id, fight_time, fight_type

    // fight_log, fight_stats
    m_mailsMap.insert(json.object().value("mail_id").toString(), content.toVariantMap());
}

void ReportManager::phpReady()
{
    QByteArray result = phpProc->readAllStandardOutput();
    QJsonDocument json = QJsonDocument::fromJson(result);
    QJsonObject data = json.object();
    if(data.contains("hitZones")) { // is a player
        QVariantMap player;
        player.insert("report_time", m_currentFightTime);
        player.insert("knight_id",data.value("id").toVariant());
        player.insert("knight_name",data.value("name").toVariant());
        player.insert("knight_level",data.value("level").toVariant());
        player.insert("strength",data.value("strength").toVariant());
        player.insert("dexterity",data.value("dexterity").toVariant());
        player.insert("endurance",data.value("endurance").toVariant());
        player.insert("luck",data.value("luck").toVariant());
        player.insert("weapon",data.value("weapon").toVariant());
        player.insert("shield",data.value("shield").toVariant());
        player.insert("damage_min",data.value("display_damageMin").toVariant());
        player.insert("damage_max",data.value("display_damageMax").toVariant());
        player.insert("armour",data.value("display_armour").toVariant());
        player.insert("experience",data.value("experience").toVariant());
        player.insert("karma",data.value("karma").toVariant());
        player.insert("gender",data.value("gender").toVariant());
        player.insert("silver",data.value("silver").toVariant());
        player.insert("rubies",data.value("rubies").toVariant());
        player.insert("hitZones",data.value("hitZones").toVariant());
        player.insert("defendZones",data.value("defendZones").toVariant());
        //player.insert("manor",data.value("premium").toVariant());
/*
        "2016-12-26 22:41:07" "duel" 0
        ReportManager::phpReady: "{
            "achievement": "",
            "activeKarmaForce": "",
            "armourFromMysticalShield": 0,
            "bonusCriticalDamage": 8,
            "bonusDamage": 0,
            "bonusSpeed": 6,
            "calculatedElementalDamage": {
                "fire": 17,
                "ice": 26,
                "poison": 18,
                "shock": 16
            },
            "calculatedElementalDamageSum": 77,
            "clanPremium": {
                "dummy": false,
                "treasureChest": false,
                "wall": false
            },
            "combatant": "defender",
            "damageDuringCombat": 0,
            "elementalDamage": {
                "fire": 21,
                "ice": 30,
                "poison": 29,
                "shock": 19
            },
            "elementalResistance": {
                "fire": 29,
                "ice": 34,
                "poison": 38,
                "shock": 25
            },
            "enemyDefeated": 7,
            "health": 3161,
            "humanIDs": [
            ],
            "involvedUsers": 1,
            "picture": "charMaleNeutral",
            "pictureID": 1,
            "premiumNormalizer": 1,
            "specialAbilities": {
                "BearPawSwipe": 16.699999999999999,
                "HorseTravelTimeReduction": 50,
                "ModifyDuelSilver": null,
                "ModifyUserStatArmour": 5,
                "ModifyWorkSalary": 10,
                "SetitemDragonslayer": 4
            },
            "treasureItem": 0,
            "type": "human",
        }"
*/
        if(data.value("type").toString() == "human") emit(playerCheck(QVariant(player)));
    } else {
        qDebug() << "ReportManager::phpReady:" << json.toJson() << "\n";
    }
}

void ReportManager::unserialize(const QString& data)
{
    phpProc->start("/usr/bin/php", QStringList() << "-r" << "echo json_encode(unserialize('"+data+"'));");
    if(!phpProc->waitForStarted()) return;

    if(!phpProc->waitForFinished()) return;
}
