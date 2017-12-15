#include "bkreportsmodule.h"
#include "bkaccount.h"
#include "bkplayer.h"
#include "bkreport.h"

#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QTimer>

#include <QDebug>




bkReportsModule::bkReportsModule(QJsonObject& defaults, bkAccount* account, QObject *parent) :
    bkModule("Reports", defaults, account, parent)
{
    connect(&m_workerProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(workerFinished(int,QProcess::ExitStatus)));
    m_workerProcess.setProgram("/usr/bin/php");

    addPath("/mail/index/menu");
    addPath("/mail/reports");
    addPath("/mail/getInbox/");
}

void bkReportsModule::restoreState(QJsonObject& state)
{
    bkModule::restoreState(state);
    setState("enable", true);
}

void bkReportsModule::startWorker()
{
    if(m_workerProcess.state() != QProcess::NotRunning) return;
    if(m_workPlayers.isEmpty()) return;

    QJsonObject worker = m_workPlayers.first().toObject();
    //qDebug() << "startWorker for" << worker.value("knight_id").toVariant().toInt();
    m_workerProcess.setArguments(QStringList() << "-r" << "echo json_encode(unserialize('"+worker.value("data").toString()+"'));");
    m_workerProcess.start();
}

void bkReportsModule::translatePlayer(const QJsonObject& player)
{
    bkPlayer* p = p_account->world()->player(player.value("id").toInt());
    QJsonObject tp;
    QJsonArray zones;
    tp.insert("knight_id",player.value("id"));
    tp.insert("knight_name",player.value("name"));
    tp.insert("knight_level",player.value("level"));
    tp.insert("silver",player.value("silver"));
    tp.insert("rubies",player.value("rubies"));
    tp.insert("karma",player.value("karma"));
    tp.insert("experience",player.value("experience"));
    tp.insert("gender",player.value("gender"));

    zones = player.value("hitZones").toArray();
    if(zones.count() == 5) tp.insert("hitZones", zones);
    if(zones.count() == 3) tp.insert("turnierHitZones", zones);
    zones = player.value("defendZones").toArray();
    if(zones.count() == 5) tp.insert("defendZones", zones);
    if(zones.count() == 3) tp.insert("turnierDefendZones", zones);

    tp.insert("strength", player.value("strength"));
    tp.insert("dexterity", player.value("dexterity"));
    tp.insert("endurance", player.value("endurance"));
    tp.insert("luck", player.value("luck"));
    tp.insert("shield", player.value("shield"));
    tp.insert("weapon", player.value("weapon"));
    tp.insert("damage_min", player.value("display_damageMin"));
    tp.insert("damage_max", player.value("display_damageMax"));
    tp.insert("armour", player.value("display_armour"));

    tp.insert("lastUpdated", player.value("lastUpdated"));
    p->setData(tp);

    //qDebug() << "translatePlayer:" << p;
}

void bkReportsModule::workerFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QJsonObject worker = m_workPlayers.first().toObject();

    //qDebug() << "bkReportsModule::workerFinished" << exitCode << exitStatus << worker.value("knight_id").toVariant().toInt() << worker.value("fight_time").toString();
    if(exitCode == 0) {
        QJsonDocument ret = QJsonDocument::fromJson(m_workerProcess.readAllStandardOutput());
        QJsonObject player = ret.object();
        int id = worker.value("knight_id").toVariant().toInt();

        if(player.value("id").toInt() != id) return;

        QString fight_time = worker.value("fight_time").toString();
        if(!checkPlayer(id, fight_time)) {
            player.insert("lastUpdated", fight_time);
            translatePlayer(player);
        }
        //qDebug() << "-->stdout:" << ret.toJson().data();
    } else {
        qDebug() << "bkReportsModule::workerFinished(?error?)" << exitCode << exitStatus;
    }

    if(!m_workPlayers.isEmpty()) m_workPlayers.removeFirst();
    QTimer::singleShot(1,this, SLOT(startWorker()));
}

bool bkReportsModule::checkPlayer(int id, QString dateTime)
{
    bkPlayer* player = p_account->player(id);
    if(!player->value("lastUpdated").isValid()) return false;

    QDateTime fightDateTime = QDateTime::fromString(dateTime, Qt::ISODate);
    QDateTime lastUpdate = QDateTime::fromString(player->value("lastUpdated").toString(), Qt::ISODate);
    if(lastUpdate < fightDateTime) return false;
    return true;
}

void bkReportsModule::parseDuel(QJsonObject mail)
{
    // ("aggressor", "aggressor_id", "defender", "defender_id", "fight_id", "fight_log", "fight_stats", "fight_time", "fight_type")
    int fight_id = mail.value("fight_id").toString().toInt();
    QString fight_time = mail.value("fight_time").toString();
    bkReport* report = p_account->world()->report(fight_id);
    if(report->isEmpty()) {
        // report_id = fight_id oder war_id
        //m_keys << "aggressor" << "aggressor_id" << "defender" << "defender_id"
               //<< "fight_id" << "fight_log" << "fight_stats" << "fight_time" << "fight_type";
        QJsonObject report_json;
        report_json.insert("fight_id", fight_id);
        report_json.insert("fight_time", fight_time);
        report_json.insert("fight_type", mail.value("fight_type").toString());
        report_json.insert("fight_log", mail.value("fight_log").toString());
        report_json.insert("fight_stats", mail.value("fight_stats").toString());

        int id = mail.value("aggressor_id").toString().toInt();
        report_json.insert("aggressor_id", id);
        if(!checkPlayer(id, fight_time)) {
            QJsonObject worker;
            worker.insert("knight_id", id);
            worker.insert("fight_time", fight_time);
            worker.insert("data", mail.value("aggressor").toString().replace("O:13:\"FighterObject\":","a:"));
            m_workPlayers.append(worker);
            QTimer::singleShot(1,this, SLOT(startWorker()));
        }

        id = mail.value("defender_id").toString().toInt();
        report_json.insert("defender_id", id);
        if(!checkPlayer(id, fight_time)) {
            QJsonObject worker;
            worker.insert("knight_id", id);
            worker.insert("fight_time", fight_time);
            worker.insert("data", mail.value("defender").toString().replace("O:13:\"FighterObject\":","a:"));
            m_workPlayers.append(worker);
            QTimer::singleShot(1,this, SLOT(startWorker()));
        }
        report->setData(report_json);
    }
    //qDebug() << "parseDuel:" << fight_id << mail.value("defender_id").toString().toInt() << fight_time;
}

void bkReportsModule::checkMail(QJsonObject mail)
{
    // ("knight_id", "mail_content", "mail_html", "mail_id", "mail_receiver", "mail_receiver_status",
    //  "mail_sender", "mail_sender_status", "mail_senttime", "mail_subject", "mail_type", "recipient_enemy_defeated",
    //  "recipient_gender", "recipient_name")

    //int mail_id = mail.value("mail_id").toString().toInt();

    int mail_type = mail.value("mail_type").toString().toInt();
/*
    qDebug() << QString("--->bkReportsModule::checkMail(%1) type %2: %3")
                .arg(mail_id)
                .arg(mail_type)
                .arg(mail.value("mail_subject").toString())
                .toLocal8Bit().data() << mail.value("mail_type").toVariant();
*/
    // (mailType == 0) Kampfbericht / Als Söldner angeheuert. / Angriffsphase / Lanzenstechen - Turnierstart
    // (mailType == 2) Ordensnachricht
    // (mailType == 3) Ordensschlacht
    //qDebug() << "mail:" << mailId << mailType << mo.value("mail_senttime").toString() << mo.value("mail_subject").toString();
    if(mail_type != 0 && mail_type != 3) return;
    QString mail_subject = mail.value("mail_subject").toString();

    QJsonObject mail_content = mail.value("mail_content").toObject();
    // Turnierstart () no mail_content
    if(mail_content.isEmpty()) {
        // war_type(raid)

        // (mail_type == 0)
        if(mail_subject == "Als Söldner angeheuert.") {
            // mo.value("mail_senttime").toString() + (24h + 9 x 8h) = clanWar-Ende
        } else if(mail_subject == "Angriffsphase") {
            // OS Runde 1.
        } else if(mail_subject == "Lanzenstechen - Turnierstart") {
            // Runde 1. in 24h
        }
        return;
    }
    if(mail_content.contains("war_id")) {
        // fightType("")
        // Ordensschlacht ("war_attacker", "war_castle", "war_defender", "war_endtime", "war_fight_data", "war_id", "war_initiator", "war_looted_silver", "war_payment_data", "war_result", "war_rounds", "war_starttime", "war_type")
        //qDebug() << QJsonDocument(mail_content).toJson().data() << "\tbkReportsModule::checkMail";
        return;
    }
    if(mail_content.contains("fight_id")) {
        // fight_type("warfight") = Kriegsduell
        // fight_type("duel") = Duell
        // fight_type("mob") = Mission
        // fight_type("group") = Gruppenmission
        // fight_type("joust") = Turnier

        //int fight_id = mail_content.value("fight_id").toString().toInt();

        QString fight_type = mail_content.value("fight_type").toString();
        if(fight_type == "duel") {
            parseDuel(mail_content);
        } else {
            //qDebug() << "anderer Report:" << fight_id << fight_type;
        }
    }
}

void bkReportsModule::mailInbox(QNetworkReply* reply)
{
    QByteArray data = reply->property("getData").toByteArray();
    QJsonDocument json = QJsonDocument::fromJson(data);
    if(json.object().value("countMails").toInt() == 0) return;

    QJsonArray mails = json.object().value("mails").toArray();
    foreach(QJsonValue value, mails) {
        checkMail(value.toObject());
    }
}

void bkReportsModule::replyFinished(QNetworkReply* reply)
{
    QString dpath = reply->url().path();
    if(dpath == "/mail/getInbox/") {
        QUrlQuery query(reply->property("postData").toString());
        if(query.hasQueryItem("inboxtype") && query.queryItemValue("inboxtype") == "reports") {
            mailInbox(reply);
        }
    } else if(dpath == "/ajax/mail/markasread/") {
        // POST:'mails[0]=61704252'
    }
}

void bkReportsModule::parse(QWebPage* page, QVariant, QVariant, QVariant)
{
    // <a id="navMessages" href="https://s12-de.battleknight.gameforge.com:443/mail/index/menu">Nachrichten<span id="devCountNewMails">1</span></a>
    QWebElement news = page->mainFrame()->findFirstElement("span#devCountNewMails");
    if(!news.isNull()) {
        qDebug() << QString("bkReportsModule::parse(has some news): %1").arg(news.toPlainText().trimmed().toInt()).toLocal8Bit().data();
    }
}

void bkReportsModule::logic(QWebPage*)
{
}

void bkReportsModule::action(QWebPage*)
{
}
