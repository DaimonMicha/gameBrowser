#include "account.h"
#include "itemmanager.h"

#include <QUrl>
#include <QDateTime>
#include <QTimerEvent>
#include <QWebFrame>
#include <QWebElement>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QDebug>


Account::Account(const QString cookie, QObject *parent) :
    QObject(parent),
    m_cookieValue(cookie),
    s_networkManager(0),
    m_cleanTimer(0),
    s_itemManager(new ItemManager)
{
    m_config.bot = false;
    m_missionsTimer = startTimer(60 * 60 * 1000);
    m_gmTimer = startTimer(60 * 60 * 1000);
}

void Account::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_cleanTimer) {
        killTimer(m_cleanTimer);
        m_cleanTimer = 0;
        reasonCleaner();
    } else if(event->timerId() == m_missionsTimer) {
    } else if(event->timerId() == m_gmTimer) {
    }
}

void Account::reasonCleaner()
{
    m_player.remove("waitDuration");
    m_player.remove("waitReason");
    m_player.remove("waitTime");
    qDebug() << "reasonCleaner...";
}

void Account::toggle(const QString option, const bool on)
{
    if(option == "account") {
        m_config.bot = on;
        if(on) {
        } else {
        }
    }
    qDebug() << "Account::toggle:" << option << on;
}

void Account::setProfile(const QVariant data)
{
    QJsonDocument json = QJsonDocument::fromVariant(data);
    QJsonObject o = json.object();

    foreach(QString key, o.keys()) {
        m_player.insert(key,o.value(key).toVariant());
    }

    if(o.contains("waitReason") && m_cleanTimer == 0) {
        m_cleanTimer = startTimer(m_player.value("waitTime").toInt()*1000+100);
    }

    qDebug() << json.toJson();// << "\n" << m_player;
}

QString Account::profile(const QString key) const
{
    QString ret;

    if(m_player.contains(key)) {
        //qDebug() << m_player.value(key).type();
        ret = m_player.value(key).toString();
    }

    return(ret);
}

void Account::setItem(const QVariant data)
{
    s_itemManager->checkItem(data);
}

void Account::loadFinished(QWebPage* page)
{
    QWebFrame* mainFrame = page->mainFrame();
    QUrl url = mainFrame->url();
    QStringList paths = url.path().split("/",QString::SkipEmptyParts);

    if(!paths.count()) return; // nothing to do, login evtl?

    if(!s_networkManager) s_networkManager = page->networkAccessManager();

    mainFrame->evaluateJavaScript("km_profile.parseData();");

    if(paths.at(0) == QString("user")) {
        if(paths.count() == 1) {
            // read inventory
            QWebElementCollection items = mainFrame->findAllElements(".inventoryItem");
            foreach(QWebElement item, items) {
                mainFrame->evaluateJavaScript("km_itemData('"+item.attribute("id")+"');");
            }
        }
    } else if(paths.at(0) == QString("world")) {
/*
<div id="mainContent" class="location cf1">
    <div id="mapBase">

      <a class="cf1CoastalFortress1 origin"></a>
      <a id="BurningVillage" class="special3 toolTip mission cf1BurningVillage" rel="#modalcf1BurningVillage"></a>
      <a id="AdversaryBridgehead" class="special3 toolTip mission cf1Bridgehead" rel="#modalcf1Bridgehead"></a>
      <a id="Coast" class="special3 toolTip mission cf1Riverbank" rel="#modalcf1Riverbank"></a>		<!-- Start Encounters -->
            </div>
*/
        if(paths.count() == 1) {
        } else {
            if(paths.at(1) == QString("location")) {
            } else if(paths.at(1) == QString("travel")) {
            }
        }
    } else if(paths.at(0) == QString("groupmission")) {
/*
 <form id="formFound" action="http://s12.de.battleknight.gameforge.com/groupmission/group" method="post">
*/
        if(paths.count() == 1) {
        } else {
            if(paths.at(1) == QString("group")) {
            }
        }
    } else if(paths.at(0) == QString("market")) {
/*
<div id="mainContent" class="marketplace work_singledout">
    <div id="marketplaceContent" class="work">
        <div class="scrollLongTall">
            <div class="cap"></div>
            <div class="innerContent clearfix">
                <p>Du hast während deiner Arbeitszeit Folgendes verdient:</p>
                <table id="workPaycheckTable">
                    <tbody><tr>
                    <td>
                        <div class="workSilverIconPaycheck"></div><em>194</em>
                    </td>
                                        <td>
                        <div id="karmaIcon" class="workKarmaIconPaycheck iconKarmaEvil"></div><em>1</em>
                    </td>
                                        </tr>
                </tbody></table>
                <p><a id="encashLink" class="button"><span>Sold abholen</span></a></p>
            </div>
            <div class="foot"></div>
        </div>
        <form id="formEncash" method="POST" action="http://s12.de.battleknight.gameforge.com/market/work">
            <input type="hidden" name="paycheck" value="encash">
        </form>
    </div>
</div>
*/
        if(paths.count() == 1) {
        } else {
            if(paths.at(1) == QString("work")) {
            } else if(paths.at(1) == QString("merchant")) {
            }
        }
    } else if(paths.at(0) == QString("duel")) {
    } else if(paths.at(0) == QString("clan")) {
    } else if(paths.at(0) == QString("manor")) {
    }
}

void Account::replyFinished(QNetworkReply* reply)
{
    QUrl url = reply->url();
    QStringList paths = url.path().split("/",QString::SkipEmptyParts);

    if(!paths.count()) return; // nothing to do, login evtl?

    if(!s_networkManager) s_networkManager = reply->manager();

    if(paths.at(0) == QString("ajax")) {
        if(paths.at(1) == QString("ajax")) {
            if(paths.at(2) == QString("getInventory")) {
                QJsonDocument json = QJsonDocument::fromJson(reply->property("getData").toByteArray());
                QJsonArray items = json.object().value("items").toArray();
                for(int i = 0; i < items.size(); ++i) {
                    setItem(items[i].toObject());
                }
            }
        }
    } else if(paths.at(0) == QString("world")) {
        if(paths.count() == 2 && paths.at(1) == QString("location")) {
            qDebug() << url.query();
        }
    }
}
