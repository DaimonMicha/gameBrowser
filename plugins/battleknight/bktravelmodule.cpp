#include "bktravelmodule.h"
#include "bkaccount.h"

#include <QNetworkReply>
#include <QUrlQuery>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

#include <QDebug>




bkTravelModule::bkTravelModule(QJsonObject& defaults, bkAccount* account, QObject *parent) :
    bkModule("Travel", defaults, account, parent)
{
    addPath("/world");
    addPath("/world/map");
    addPath("/world/location");
    addPath("/world/travel");
}

void bkTravelModule::restoreState(QJsonObject& state)
{
    bkModule::restoreState(state);
    setState("state", mode_unknown);
    setState("enable", true);
}

void bkTravelModule::reply_startTravel(QNetworkReply* reply)
{
    QUrlQuery query(reply->property("postData").toString());
    // travelwhere=HarbourOne&travelhow=cog&travelpremium=0
    if(!query.hasQueryItem("travelwhere")) return;
    qDebug() << "bkTravelModule::startTravel()" << query.queryItemValue("travelwhere");
}

void bkTravelModule::replyFinished(QNetworkReply* reply)
{
    QString dpath = reply->url().path();

    if(dpath == "/world/startTravel") {
        reply_startTravel(reply);
    } else if(dpath == "/world/abortTravel") {
    }
}

void bkTravelModule::parse_worldLocation(QWebPage* page)
{
    QWebElement document = page->mainFrame()->documentElement();
    QWebElement main = document.findFirst("#mainContent");
    if(main.isNull() || !main.hasClass("location")) return;

    // Location
    QVariant location = main.classes().last();
    setState("location", location.toString());
    p_account->setStatus("location", location);

    // Missions
    //
    QWebElementCollection links = main.findAll("a");
    foreach(QWebElement link, links) {
        if(link.hasClass("mission")) {
            // Dragon-Event
            // neue strategie: jage den großen Drachen!
            if(link.attribute("id") == "DragonEventGreyDragon") {
            } else if(link.attribute("id") == "DragonEventGreatDragon") {
            }
            qDebug() << "bkTravelModule::parse_worldLocation(Mission)" << link.attribute("id") << link.classes();
        }
    }
}

void bkTravelModule::checkEvents(QWebElement document)
{
    /*
    <div id="mainContent" class="map world18">
        <div id="mapBase">
            <div id="DragonIcon" class="TradingPostFour"></div>
    */
    QWebElement event = document.findFirst("#DragonIcon");
    if(!event.isNull()) {
        qDebug() << "bkTravelModule::checkEvents(DragonEvent)" << event.classes();
        setState("event", "DragonEvent");
    }
    // andere Events checken?

    if(event.isNull()) {
        setState("event", QJsonValue()); // löschen?
    }
}

void bkTravelModule::parse_worldMap(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();
    // <div id="mainContent" class="map world18">
    QWebElement main = doc.findFirst("#mainContent");
    if(!main.hasClass("map")) return;

    checkEvents(doc);

    QWebElementCollection towns = main.findAll("div.askpoint");
    foreach(QWebElement town, towns) {
        QWebElement tName = town.findFirst("div.cap");
        QString out = tName.toPlainText().trimmed();
        //qDebug() << out;
    }
}

void bkTravelModule::parse_worldTravel(QWebPage* page)
{
    QWebElement doc = page->mainFrame()->documentElement();
    // <div id="mainContent" class="map world18">
    QWebElement main = doc.findFirst("#mainContent");

    checkEvents(doc);

    // <div class="locationName">
    // <div id="h2" class="askpoint closed" onmouseover="highlightRoutes('HarbourTwo')">
/*

<div id="v4" class="askpoint closed" onmouseover="highlightRoutes('VillageFour')">
    <div class="marker dest"></div>
    <div id="v4_toolTip" class="travelTip travelTip_opposite" style="display: none;">
        <div class="glass"></div>
        <div class="glassIcon"></div>
        <div class="cap"> Jarow </div>
        <div class="innerContent clearfix">
            <table id="travelTipTable" class="ordenTable travelTable" border="0" cellspacing="0" cellpadding="0">
              <colgroup>
                <col class="travelTable01" width="35">
                <col class="travelTable02" width="134">
                <col class="travelTable03" width="64">
                <col class="travelTable04" width="94">
                <col class="travelTable05" width="125">
              </colgroup>
              <thead>
                <tr>
                  <th class="travelTable01" scope="col">&nbsp;

                  </th>
                  <th class="travelTable02" scope="col">
                    <div class="thIcon iconTravelTime toolTip">
                    </div>
                  </th>
                  <th class="travelTable03" scope="col">
                    <div class="thIcon iconTravelDuration toolTip">
                    </div>
                  </th>
                  <th class="travelTable04" scope="col">
                  </th>
                  <th class="travelTable05" scope="col">
                  </th>
                </tr>
              </thead>
                <tbody>
                    <tr>
                        <td class="travelTable01"><div class="thIcon iconHorse toolTip"></div></td>
                        <td class="travelTable02 toolTip"> sofort </td>
                        <td class="travelTable03 toolTip"> 00h 13m </td>
                        <td class="travelTable04"> </td>
                        <td class="travelTable05">
                            <div class="inlineBtnWrapper blockCenter">
                                <a class="button boxed tooltip " onclick="startTravel('VillageFour', 'horse', new Element(this), false);">
                                    <span> los </span>
                                </a>
                            </div>
                        </td>
                    </tr>

                    <tr>
                      <td class="travelTable01">
                        <div class="thIcon iconCaravan toolTip">
                        </div>
                      </td>
                      <td class="travelTable02 toolTip">
                        sofort
                      </td>
                      <td class="travelTable03 toolTip">
                        sofort
                      </td>
                      <td class="travelTable04 toolTip toolTipCashForCaravan">
                        <div class="icon iconRuby">
                        </div>
                        5
                      </td>
                      <td class="travelTable05">
                        <div class="inlineBtnWrapper blockCenter">
                          <a class="button boxed tooltip " rel="special" onclick="startTravel('VillageFour', 'caravan', new Element(this), true);">
                            <span>
                              los
                            </span>
                          </a>
                        </div>
                      </td>
                    </tr></tbody></table>
        </div>
        <div class="foot"></div>
    </div>
    <div class="locationName">
        <div class="locationName_top"></div>
        <div class="locationName_content">Jarow</div>
    </div>
</div>

<div id="h1_toolTip" class="travelTip travelTip_opposite" style="display: none;">
        <div class="glass">
                  </div>
                  <div class="glassIcon">
                  </div>
        <div class="cap">
          Waile
        </div>
        <div class="innerContent clearfix">
            <table id="travelTipTable" class="ordenTable travelTable" border="0" cellspacing="0" cellpadding="0">
              <colgroup>
                <col class="travelTable01" width="35">
                <col class="travelTable02" width="134">
                <col class="travelTable03" width="64">
                <col class="travelTable04" width="94">
                <col class="travelTable05" width="125">
              </colgroup>
              <thead>
                <tr>
                  <th class="travelTable01" scope="col">&nbsp;

                  </th>
                  <th class="travelTable02" scope="col">
                    <div class="thIcon iconTravelTime toolTip">
                    </div>
                  </th>
                  <th class="travelTable03" scope="col">
                    <div class="thIcon iconTravelDuration toolTip">
                    </div>
                  </th>
                  <th class="travelTable04" scope="col">
                  </th>
                  <th class="travelTable05" scope="col">
                  </th>
                </tr>
              </thead>      <tbody><tr>
                      <td class="travelTable01">
                        <div class="thIcon iconCog toolTip">
                        </div>
                      </td>
                      <td class="travelTable02 toolTip">
                        sofort
                      </td>
                      <td class="travelTable03 toolTip">
                        00h 12m
                      </td>
                      <td class="travelTable04 toolTip toolTipCashForCog">
                        <div class="icon iconSilverSmlGlow">
                        </div>
                        953
                      </td>
                      <td class="travelTable05">
                        <div class="inlineBtnWrapper blockCenter">
                          <a class="button boxed tooltip " rel="special" onclick="startTravel('HarbourOne', 'cog', new Element(this), false);">
                            <span>
                              los
                            </span>
                          </a>
                        </div>
                      </td>
                    </tr>      <tr>
                      <td class="travelTable01">
                        <div class="thIcon iconCog toolTip">
                        </div>
                      </td>
                      <td class="travelTable02 toolTip">
                        sofort
                      </td>
                      <td class="travelTable03 toolTip">
                        sofort
                      </td>
                      <td class="travelTable04 toolTip toolTipCashForCog">
                        <div class="icon icon iconRuby">
                        </div>
                        5
                      </td>
                      <td class="travelTable05">
                        <div class="inlineBtnWrapper blockCenter">
                          <a class="button boxed tooltip " rel="special" onclick="startTravel('HarbourOne', 'cog', new Element(this), true);">
                            <span>
                              los
                            </span>
                          </a>
                        </div>
                      </td>
                    </tr></tbody></table></div>
          <div class="foot">
          </div>
    </div>
    <div class="locationName">
        <div class="locationName_top"></div>
        <div class="locationName_content">Waile</div>
    </div>
</div>

<div id="f2" class="askpoint closed">
    <div class="marker origin"></div>
    <div id="f2_toolTip" class="travelTip travelTip_opposite" style="display: none;">
        <div class="glass"></div>
        <div class="glassIcon"></div>
        <div class="cap"> Segur </div>
        <div class="innerContent clearfix">Du befindest dich in diesem Ort.</div>
        <div class="foot"></div>
    </div>
    <div class="locationName">
        <div class="locationName_top"></div>
        <div class="locationName_content">Segur</div>
    </div>
</div>

*/
    QWebElementCollection towns = main.findAll("div.askpoint");
    foreach(QWebElement town, towns) {
        QWebElement tName = town.findFirst("div.cap");
        QString out = tName.toPlainText().trimmed();

        // Location
        QWebElement marker = town.findFirst("div.marker");
        if(!marker.isNull() && marker.hasClass("origin")) {
            QVariant location = town.attribute("id");
            setState("location", location.toString());
            p_account->setStatus("location", location);
            out.append(", hier stehe ich (" + location.toString() + ")");
        }

        // hier können wir hinreisen
        if(town.hasAttribute("onmouseover")) {
            out.append(": "+town.attribute("onmouseover"));
        }
        qDebug() << "bk" + name() + "Module::parse_worldTravel()" << out;
    }

    // <div id="h2tp4" class="path"></div><div id="tp4f2" class="path"></div><div id="v4tp4" class="path"></div>
    QWebElementCollection routes = main.findAll("div.path");
}

void bkTravelModule::parse(QWebPage* page, QVariant titleTimer, QVariant, QVariant)
{
    QString dpath = page->mainFrame()->url().path().toLower();
    if(!hasPath(dpath)) return;

    if(dpath == "/world/map") {
        parse_worldMap(page);
    } else if(dpath == "/world/travel") {
        if(!titleTimer.isValid()) parse_worldTravel(page);
    } else if(dpath == "/world" || dpath == "/world/location") {
        if(!titleTimer.isValid()) parse_worldLocation(page);
    }

    updated();
    if(active()) emit jobDone();
}

void bkTravelModule::logic(QWebPage*)
{
}

void bkTravelModule::action(QWebPage*)
{
}
