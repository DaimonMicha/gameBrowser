#include "reportmanager.h"
#include "account.h"

#include <QJsonObject>
#include <QJsonArray>

#include <QProcess>
#include <QTimer>

#include <QDebug>




ReportManager::ReportManager(QObject *parent) :
    QObject(parent),
    s_workingTimer(new QTimer),
    phpProc(new QProcess),
    isWorking(false)
{
    s_workingTimer->setSingleShot(true);
    connect(s_workingTimer, SIGNAL(timeout()), this, SLOT(processNextReport()));
    connect(phpProc, SIGNAL(readyReadStandardOutput()), this, SLOT(phpReady()));
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
    QString fightType = content.value("fight_type").toString();
    if(fightType == "duel" || fightType == "joust") {
        m_workList.append(json);
        if(!isWorking && !s_workingTimer->isActive()) s_workingTimer->start(100);
    }
    return;

    //QJsonDocument debug(content);
    qDebug() << "ReportManager::checkReport:"
             << json.object().value("mail_id").toString()
             << content.value("fight_time").toString()
             << content.value("fight_type").toString()
             << content.value("fight_id").toString()
             << json.object().keys();

}

void ReportManager::processNextReport()
{
    if(m_workList.isEmpty() || isWorking) return;
    isWorking = true;
    QJsonDocument json = m_workList.takeFirst();
    QJsonObject content = json.object().value("mail_content").toObject();
    // fight_id, fight_time, fight_type
    m_currentFightType = content.value("fight_type").toString();
    m_currentFightTime = content.value("fight_time").toString();
    if(content.contains("aggressor")) {
        unserialize(content.value("aggressor").toString().replace("O:13:\"FighterObject\":","a:"));
        content.insert("aggressor", m_currentDocument.object());
        m_currentDocument = QJsonDocument();
    }
    if(content.contains("defender")) {
        unserialize(content.value("defender").toString().replace("O:13:\"FighterObject\":","a:"));
        content.insert("defender", m_currentDocument.object());
        m_currentDocument = QJsonDocument();
    }
    if(content.contains("fight_log")) {
        unserialize(content.value("fight_log").toString());
        content.insert("fight_log", m_currentDocument.array());
        m_currentDocument = QJsonDocument();
    }
    if(content.contains("fight_stats")) {
        unserialize(content.value("fight_stats").toString());
        content.insert("fight_stats", m_currentDocument.object());
        m_currentDocument = QJsonDocument();
    }
    m_mailsMap.insert(json.object().value("mail_id").toString(), content.toVariantMap());

    /*
        QJsonDocument debug(content);
        qDebug() << "ReportManager::processNextReport()" << debug.toJson();
    */
    isWorking = false;
    if(!m_workList.isEmpty()) s_workingTimer->start();
}

/*
 * types: duel, joust
 */
QVariantMap ReportManager::lastReport(int maxCount, QString type) const
{
    QVariantMap ret;
    int c = 1;

    QMap<QString, QVariant>::const_iterator i = m_mailsMap.constEnd();
    while(i != m_mailsMap.constBegin()) {
        --i;
        if(type == "*") {
            ret.insert(i.key(), i.value());
            ++c;
        } else {
            QString isType = i.value().toMap().value("fight_type").toString();
            if(type == isType) {
                ret.insert(i.key(), i.value());
                ++c;
            }
        }
        if(c > maxCount) break;
    }

    //qDebug() << "\t" << type << ret.count() << i.value().toMap().value("fight_type").toString();
    return(ret);
}

void ReportManager::phpReady()
{
    QByteArray result = phpProc->readAllStandardOutput();
    QJsonDocument json = QJsonDocument::fromJson(result);
    m_currentDocument = json;
    if(json.isArray()) return;
    if(!json.isObject()) return;
    QJsonObject data = json.object();
    if(data.contains("type") && data.value("type").toString() == QString("human")) { // is a player
        QVariantMap player;
        player.insert("report_type", m_currentFightType);
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
        QJsonArray cleanHits;
        foreach(QJsonValue item, data.value("hitZones").toArray()) {
            cleanHits.append(QJsonValue(item.toString().toInt()));
        }
        QJsonDocument hitZones(cleanHits);
        player.insert("hitZones",hitZones.toJson(QJsonDocument::Compact));
        QJsonArray cleanDefs;
        foreach(QJsonValue item, data.value("defendZones").toArray()) {
            cleanDefs.append(QJsonValue(item.toString().toInt()));
        }
        QJsonDocument defendZones(cleanDefs);
        player.insert("defendZones",defendZones.toJson(QJsonDocument::Compact));
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
        emit(playerCheck(QVariant(player)));
    } else {
        if(data.contains("aggressor") && data.contains("defender")) return; // fight_log
        qDebug() << "ReportManager::phpReady (unknown):" << json.toJson(QJsonDocument::Compact);
    }
}

void ReportManager::unserialize(const QString& data)
{
    phpProc->start("/usr/bin/php", QStringList() << "-r" << "echo json_encode(unserialize('"+data+"'));");
    if(!phpProc->waitForStarted()) {
        // error-handling!
        //return;
    }

    if(!phpProc->waitForFinished()) {
        // error-handling!
        //return;
    }
}
