#include "account.h"
#include "ogame.h"

#include <QUrl>
#include <QDateTime>
#include <QWebFrame>
#include <QWebElement>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>

#include <QDebug>


Account::Account(const QString cookie, QObject *parent) :
    QObject(parent),
    m_cookieValue(cookie),
    s_networkManager(0),
    m_currentPlayer(""),
    m_currentPlanet("")
{
    OGame* plugin = qobject_cast<OGame *>(parent);
    if(plugin) {
        QString script;
        plugin->readDataFile("constants.json",script);
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(script.toUtf8(),&err);
        if(doc.isNull()) qDebug() << "JSON::Error," << err.errorString();
        m_constants = doc.object();
        //qDebug() << "constants:" << doc.toJson(QJsonDocument::Compact);
    }
}

void Account::toggle(const QString option, const bool soll)
{
    bool ist = isActive(option);

    if(option == "enableAccount") {
        if(soll) {
        } else {
        }
    }

    if(ist != soll) {
        m_botOptions.insert(option, soll);
        qDebug() << parent()->metaObject()->className() << "Account::toggle:" << option << "von" << ist << "zu" << soll;
    }
}

QJsonObject Account::parseFleet(QWebElement& fleet)
{
    QString fid = fleet.attribute("id").remove(0,5);

    QJsonObject fleetObject = getFleet(fid);
    fleetObject.insert("id", fid.toInt());
    fleetObject.insert("type", fleet.attribute("data-mission-type").toInt());
    fleetObject.insert("isReturn", fleet.attribute("data-return-flight"));
    fleetObject.insert("arrival", fleet.attribute("data-arrival-time").toInt());

    QWebElement coords;
    QWebElement planet;
    QWebElement details = fleet.findFirst("table.fleetinfo");

    coords = fleet.findFirst(".originCoords");
    planet = fleet.findFirst(".originPlanet");
    fleetObject.insert("originCoords", coords.toPlainText());
    fleetObject.insert("originPlanet", planet.toPlainText());

    coords = fleet.findFirst(".destinationCoords");
    planet = fleet.findFirst(".destinationPlanet");
    fleetObject.insert("destCoords", coords.toPlainText());
    fleetObject.insert("destPlanet", planet.toPlainText());

    QJsonArray techs = m_constants.value("techs").toArray();
    QJsonArray shipsList, cargoList;
    bool cargo = false;
    foreach(QWebElement row, details.findAll("tr")) {
        QWebElement fc = row.firstChild();
        if(fc.tagName() == "TH") {
            if(fc.toPlainText() == "Ladung:") cargo = true;
            continue;
        }
        QString key = fc.toPlainText().trimmed();
        if(key.isEmpty()) continue;
        key.chop(1);
        int sid = 0;
        int count = 0;
        if(cargo) { // cargo
            count = fc.nextSibling().toPlainText().trimmed().replace(",","").replace(".","").toLong();
            //qDebug() << "cargo:" << key << count;
            QJsonObject tc;
            tc.insert("type", key);
            tc.insert("amount", count);
            cargoList.append(tc);
        } else { // ships
            foreach(QJsonValue tech, techs) {
                if(QString::compare(key, tech.toObject().value("name").toString()) == 0) {
                    sid = tech.toObject().value("id").toInt();
                    count = fc.nextSibling().toPlainText().trimmed().toInt();
                    break;
                }
            }
            //qDebug() << "ship:" << sid << key << count;
            QJsonObject ts;
            ts.insert("id", sid);
            ts.insert("amount", count);
            shipsList.append(ts);
        }
    }
    fleetObject.insert("cargo", cargoList);
    fleetObject.insert("ships", shipsList);
    m_fleets.insert(fid, fleetObject);
    return(fleetObject);
}

QJsonObject Account::getFleet(QString& fid)
{
    QDateTime now = QDateTime::currentDateTime();
    foreach(QJsonValue row, m_fleets) {
        qint64 ts = row.toObject().value("arrival").toInt();
        QDateTime arrival = QDateTime::fromMSecsSinceEpoch(ts * 1000);
        if(arrival < now) {
            qDebug() << "\t (arrival < now) in getFleet" << arrival.toMSecsSinceEpoch() << now.toMSecsSinceEpoch();
            m_fleets.remove(QString("%1").arg(row.toObject().value("id").toInt()));
        }
    }

    if(m_fleets.contains(fid)) return(m_fleets.value(fid).toObject());
    // ToDo: new Fleet, start a timer
    qDebug() << "read new Fleet" << fid.toInt();
    return(QJsonObject());
}

int Account::fleetCargo(QJsonObject& fleet)
{
    int ret = 0;
    QJsonArray cargo = fleet.value("cargo").toArray();
    foreach(QJsonValue row, cargo) {
        QJsonObject c = row.toObject();
        ret += c.value("amount").toInt();
    }

    return(ret);
}

int Account::fleetCapacity(QJsonObject& fleet)
{
    int ret = 0;
    QJsonArray techs = m_constants.value("techs").toArray();
    QJsonArray ships = fleet.value("ships").toArray();
    foreach(QJsonValue row, ships) {
        foreach(QJsonValue tech, techs) {
            if(tech.toObject().value("id").toInt() == row.toObject().value("id").toInt()) {
                QJsonObject data = tech.toObject().value("data").toObject();
                ret += data.value("capacity").toInt() * row.toObject().value("amount").toInt();
                break;
            }
        }
    }
    return(ret);
}

QJsonObject Account::getPlanet(QString& pid)
{
    if(m_planets.contains(pid)) return(m_planets.value(pid).toObject());
    return(QJsonObject());
}

QJsonObject Account::parsePlanets(QWebFrame* mainFrame)
{
    QJsonObject planetObject;

    QMultiMap<QString, QString> meta = mainFrame->metaData();
    if(!meta.contains("ogame-planet-id")) return(planetObject);
    m_currentPlanet = meta.value("ogame-planet-id");
    planetObject = getPlanet(m_currentPlanet);
    planetObject.insert("id", m_currentPlanet.toInt());
    planetObject.insert("player", m_currentPlayer.toInt());
    planetObject.insert("name", meta.value("ogame-planet-name"));
    planetObject.insert("type", meta.value("ogame-planet-type"));
    planetObject.insert("coords", meta.value("ogame-planet-coordinates"));
    QJsonObject res;
    QWebElement r = mainFrame->documentElement().findFirst("#resources_metal");
    res.insert("metal", r.toPlainText().replace(".",""));
    r = mainFrame->documentElement().findFirst("#resources_crystal");
    res.insert("crystal", r.toPlainText().replace(".",""));
    r = mainFrame->documentElement().findFirst("#resources_deuterium");
    res.insert("deuterium", r.toPlainText().replace(".",""));
    planetObject.insert("resources", res);

    m_planets.insert(m_currentPlanet, planetObject);

    QWebElement plist = mainFrame->documentElement().findFirst("#planetList");
    foreach(QWebElement pElement, plist.findAll(".smallplanet")) {
        QString pid = pElement.attribute("id").remove(0,7);
        QJsonObject planet = getPlanet(pid);
        QWebElement anker = pElement.firstChild();
        planet.insert("id", pid.toInt());
        planet.insert("player", m_currentPlayer.toInt());
        planet.insert("name", anker.findFirst("span.planet-name").toPlainText());
        planet.insert("coords", anker.findFirst("span.planet-koords").toPlainText());

        QString title = anker.attribute("title");
        if(!title.isEmpty()) {
            title = title.split("<br>").at(0).split("<br/>").at(1).split(" ").at(1);
            title = title.replace("(","").replace(")","");
            title = title.split("/").at(1);
            planet.insert("size", title.toInt());
        }
        m_planets.insert(pid, planet);
    }

    return(planetObject);
}

void Account::loadFinished(QWebPage* page)
{
    if(!s_networkManager) s_networkManager = page->networkAccessManager();
    QWebFrame* mainFrame = page->mainFrame();
    QUrl url = mainFrame->url();
    QStringList paths = url.path().split("/",QString::SkipEmptyParts);

    if(!paths.count()) {
        //qDebug() << "\t" << "Login?";
        QList<QNetworkCookie> cookies = s_networkManager->cookieJar()->cookiesForUrl(url);

        return; // nothing to do, login evtl?
    }

    QUrlQuery q(url);
    if(!q.hasQueryItem("page")) return;

    OGame* plugin = qobject_cast<OGame *>(parent());
    if(plugin) {
        QString script;
        plugin->readDataFile("checkscript.js",script);
        mainFrame->evaluateJavaScript(script);
    } else {
        return;
    }

    QMultiMap<QString, QString> meta = mainFrame->metaData();
    if(!meta.contains("ogame-player-id")) return;
    m_currentPlayer = meta.value("ogame-player-id");

    QJsonObject planet = parsePlanets(mainFrame);

    //qDebug() << currentPlanet;
    QString pg = q.queryItemValue("page");
    if(pg == QString("movement")) {
        foreach(QWebElement fleet, mainFrame->documentElement().findAll(".fleetDetails")) {
            QJsonObject fleetObject = parseFleet(fleet);
            //qint64 ts = fleetObject.value("arrival").toInt();
            //QDateTime arrival = QDateTime::fromMSecsSinceEpoch(ts * 1000);
            //qDebug() << "\t Flotte:" << fleetObject.value("id").toInt() << arrival.toString() << QString("%1/%2").arg(fleetCargo(fleetObject)).arg(fleetCapacity(fleetObject)).toLocal8Bit().constData();
        }
    }

    QJsonDocument debug(planet);
    qDebug() << "\t currentPlanet:" << debug.toJson(QJsonDocument::Indented).constData();

}

void Account::replyFinished(QNetworkReply* reply)
{
    if(!s_networkManager) s_networkManager = reply->manager();
    QUrl url = reply->url();
    QStringList paths = url.path().split("/",QString::SkipEmptyParts);

    if(!paths.count()) {
        return; // nothing to do
    }

/*

"[2017-01-31 11:14:41]  OGame::replyFinished (/game/index.php?page=fleetcheck&ajax=1&espionage=0), POST:'galaxy=4&system=334&planet=7&type=1&recycler=1'"
"[2017-01-31 11:08:38]  OGame::replyFinished (/game/index.php?page=fleet3), POST:'type=1&mission=0&union=0&am202=10&am204=10&am205=10&galaxy=5&system=288&position=16&acsValues=-&speed=10'"
"[2017-01-31 10:28:55]  OGame::replyFinished (/game/index.php?page=movement),
POST:'holdingtime=1&expeditiontime=1&token=a143e9adde532ca9a923feac58e8d125
    &galaxy=4&system=334&position=7
    &type=1&mission=4&union2=0&holdingOrExpTime=0&speed=1&acsValues=-&prioMetal=1&prioCrystal=2&prioDeuterium=3
    &am203=1&am204=180&am206=27&am207=3&am209=2
    &metal=0&crystal=0&deuterium=1500'"

*/

    if(paths.at(0) == QString("game")) {
        QUrlQuery q(url);
        if(!q.hasQueryItem("page")) return; // || !q.hasQueryItem("ajax")
        QString pg = q.queryItemValue("page");
        if(pg == QString("eventList")) {
            QWebElement listDiv;
            listDiv.setPlainText(reply->property("getData").toString());
            qDebug() << "\t (eventList)" << listDiv.toPlainText();
        } else if(pg == QString("fetchEventbox")) {
            QWebElement listDiv;
            listDiv.setPlainText(reply->property("getData").toString());
            qDebug() << "\t (fetchEventbox)" << listDiv.toPlainText();
        } else if(pg == QString("fleet2")) {
            QUrlQuery pq(reply->property("getData").toString());
            //qDebug() << "\t (fleet2)" << pq.toString();
        } else if(pg == QString("movement")) {
            QUrlQuery pq(reply->property("postData").toString());
            if(pq.hasQueryItem("token")) {
                qDebug() << "\t (movement)" << pq.queryItemValue("token");
            }
        }
    } else if(paths.at(0) == QString("socket.io")) {
        //qDebug() << "\t" << reply->property("getData").toByteArray();
    }

}
