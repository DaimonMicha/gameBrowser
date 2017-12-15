#include "bkhighscoremodule.h"
#include "bkaccount.h"
#include "bkworld.h"

#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>




bkHighscoreModule::bkHighscoreModule(QJsonObject& defaults, bkAccount* account, QObject *parent) :
    bkModule("Highscore", defaults, account, parent)
{
    addPath("/highscore/");
}

void bkHighscoreModule::restoreState(QJsonObject& state)
{
    bkModule::restoreState(state);
    setState("enable", true);
}

void bkHighscoreModule::readKnightRow(QWebElement row) {
    QJsonObject knight;
    bkWorld* world = p_account->world();
    QWebElementCollection hsCols = row.findAll("td");

    foreach(QWebElement col, hsCols) {
        if(col.classes().contains("highscore02")) {
            if(col.firstChild().classes().contains("iconKarmaEvil")) knight.insert("knight_course",QJsonValue(false));
            else knight.insert("knight_course",QJsonValue(true));
        }
        if(col.classes().contains("highscore03")) {
            QWebElementCollection links = col.findAll("a");

            QWebElementCollection::iterator links_it = links.begin();
            QUrl url = (*links_it).attribute("href");
            if(!url.path().contains("/profile/") && links_it != links.end()) {
                ++links_it;
                url = (*links_it).attribute("href");
            }
            QStringList upath = url.path().split("/",QString::SkipEmptyParts);
            if(upath.at(1) == "profile") {
                knight.insert("knight_id", QJsonValue(upath.at(2).toInt()));
                QString name = links.at(0).toPlainText();
                int pos = name.indexOf(' ');
                knight.insert("knight_rang", QJsonValue(name.left(pos)));
                knight.insert("knight_name", QJsonValue(name.mid(pos+1)));

                if(links_it+(1) != links.end()) {
                    ++links_it;
                    url = (*links_it).attribute("href");
                    upath = url.path().split("/",QString::SkipEmptyParts);
                    if(upath.count() > 4) {
                        knight.insert("clan_id", QJsonValue(upath.at(2).toInt()));
                        //clanManager
                        bkClan* clan = world->clan(knight.value("clan_id").toInt());
                        QJsonObject co;
                        QString tag = (*links_it).toPlainText().trimmed();
                        tag = tag.mid(1); tag.chop(1);
                        co.insert("clan_id", QJsonValue(upath.at(2).toInt()));
                        co.insert("clan_tag", QJsonValue(tag));
                        clan->setData(co);
                    }
                }
            }

            //qDebug() << "bkHighscoreModule::readKnightRow(links.count)" << links.count() << upath;//QJsonDocument(knight).toJson().data();
        }
        if(col.classes().contains("highscore04")) {
            knight.insert("knight_level", QJsonValue(col.toPlainText().trimmed().toInt()));
        }
        if(col.classes().contains("highscore05")) {
            knight.insert("loot_won", QJsonValue(col.toPlainText().trimmed().replace(".","").toInt()));
        }
        if(col.classes().contains("highscore06")) {
            knight.insert("fights", QJsonValue(col.toPlainText().trimmed().replace(".","").toInt()));
        }
        if(col.classes().contains("highscore07")) {
            knight.insert("fights_won", QJsonValue(col.toPlainText().trimmed().replace(".","").toInt()));
        }
        if(col.classes().contains("highscore08")) {
            knight.insert("fights_lose", QJsonValue(col.toPlainText().trimmed().replace(".","").toInt()));
        }
    }
    bkPlayer* p = world->player(knight.value("knight_id").toInt());
    p->setData(knight);
}

void bkHighscoreModule::parse(QWebPage* page, QVariant, QVariant, QVariant)
{
    QString dpath = page->mainFrame()->url().path().toLower();
    if(!hasPath(dpath)) return;

    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement hsTable = doc.findFirst("#highscoreTable");
    if(hsTable.isNull()) return;
    QWebElement hsBody = hsTable.findFirst("tbody");
    QWebElementCollection hsRows = hsBody.findAll("tr");
    foreach(QWebElement hsRow, hsRows) {
        if(hsRow.classes().contains("userSeperator")) continue;
        readKnightRow(hsRow);
    }
}

void bkHighscoreModule::logic(QWebPage*)
{
}

void bkHighscoreModule::action(QWebPage*)
{
}

void bkHighscoreModule::view(QWebPage*)
{
}
