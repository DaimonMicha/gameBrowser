#include "accountui.h"
#include "account.h"

#include <QWebPage>
#include <QWebFrame>
#include <QTimerEvent>

#include <QDebug>





accountUI::accountUI(Account *account, QObject *parent) :
    QObject(parent),
    s_account(account),
    m_workingTimer(0),
    m_animateTimer(0)
{
    QWebPage* page = qobject_cast<QWebPage *>(parent);
    if(!page) return;
    connect(page->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(stopTimer()));
}

accountUI::~accountUI()
{
    //qDebug() << "~accountUI...";
}

void accountUI::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_workingTimer) check();
}

void accountUI::stopTimer()
{
    if(m_workingTimer) killTimer(m_workingTimer);
    m_workingTimer = 0;

    //qDebug() << "---accountUI::stopTimer";
}

void accountUI::check()
{
    foreach(QWebElement el, m_checkerList) {
        bool soll = s_account->isActive(el.attribute("id"));
        bool ist = el.evaluateJavaScript(QLatin1String("this.checked")).toBool();
        if(soll != ist) {
            el.evaluateJavaScript(QLatin1String("this.checked")+QString("=%1;").arg(soll));
        }
    }
    foreach(QWebElement el, m_statusList) {
        QString soll = s_account->status(el.attribute("id")).toString();
        QString ist = el.toPlainText();
        if(soll != ist) {
            el.setPlainText(soll);
            //if(!ist.isEmpty()) el.evaluateJavaScript(QLatin1String("this.highlight();"));
        }
    }
    foreach(QWebElement el, m_profileList) {
        QString soll = s_account->profile(el.attribute("id"));
        if(el.attribute("id") == "location") {
            QWebPage* page = qobject_cast<QWebPage *>(parent());
            if(!page || soll.isEmpty()) return;
            soll = page->mainFrame()->evaluateJavaScript("km_locations[\""+soll+"\"].title;").toString();
        }
        QString ist = el.toPlainText();
        if(soll != ist) {
            el.setPlainText(soll);
            //if(!ist.isEmpty()) el.evaluateJavaScript(QLatin1String("this.highlight();"));
        }
    }
}

void accountUI::inject()
{
    // avoid double-loading of the profile-page
    if(m_workingTimer) return;

    m_checkerList.clear();
    m_statusList.clear();
    m_profileList.clear();

    QWebElement missions = addModule("Missions", "Missionen", "darf ich automatisch Missionen machen?");
    addProfileText(missions,"missionPoints","Missionspunkte:");
    addLocationText(missions,"location","<tr><td></td><td class=\"kmHeader\"><span id=\"location\"></span></td></tr>");
    //Duelle!
    QWebElement gm = addModule("GM", "GM", "darf ich automatisch Gruppen-Missionen machen?");
    addProfileText(gm,"gmPoints","Missionspunkte:");
    QWebElement clanwar = addModule("ClanWar", "Ordenskrieg", "darf ich beim Ordenskrieg mitmischen?");
    addStatusText(clanwar,"battle_round","Runde:");
    QWebElement turnier = addModule("Turnier", "Turnier", "darf ich beim Turnier mitmischen?");
    addStatusText(turnier,"turnier_round","Runde:");

    check();
    m_workingTimer = startTimer(250);
    //qDebug() << "it works!" << s_account->profile("knight_id");
}

QWebElement accountUI::addModule(QString moduleId, QString moduleName, QString moduleTitle)
{
    QWebPage* page = qobject_cast<QWebPage *>(parent());
    if(!page) return(QWebElement());

    QWebElement doc = page->mainFrame()->documentElement();
    QWebElement tmpl = doc.findFirst("#ModulTemplate");
    QWebElement module = tmpl.clone();
    QWebElement target = doc.findFirst("#accountPlugin");
    module.setAttribute("id", moduleId.toLower()+"Module");
    target.appendInside(module);
    tmpl = module.findFirst("#enableModule");
    tmpl.setAttribute("id", "enable"+moduleId);
    tmpl.evaluateJavaScript("this.addEventListener('click', function(){ account.toggle(this.id, this.checked); });");
    tmpl.setAttribute("title", moduleTitle);
    m_checkerList.append(tmpl);

    tmpl = module.findFirst("label");
    tmpl.setAttribute("for", "enable"+moduleId);
    if(moduleName.isEmpty()) moduleName = moduleId;
    tmpl.setPlainText(moduleName);
    tmpl.setAttribute("title", moduleTitle);
    module.removeClass("nodisplay");
    return(module);
}

void accountUI::addStatusText(QWebElement module, QString key, QString label)
{
    QWebElement target = module.findFirst("tbody");
    target.appendInside("<tr><td colspan=\"2\">"+label+"&nbsp;<span id=\""+key+"\"></span></td></tr>");
    m_statusList.append(target.findFirst("#"+key));
}

void accountUI::addProfileText(QWebElement module, QString key, QString label)
{
    QWebElement target = module.findFirst("tbody");
    target.appendInside("<tr><td colspan=\"2\">"+label+"&nbsp;<span id=\""+key+"\"></span></td></tr>");
    m_profileList.append(target.findFirst("#"+key));
}

void accountUI::addLocationText(QWebElement module, QString key, QString tmpl)
{
    QWebElement target = module.findFirst("tbody");
    if(tmpl.isEmpty()) target.appendInside("<tr><td colspan=\"2\"><span id=\""+key+"\"></span></td></tr>");
    else target.appendInside(tmpl);
    m_profileList.append(target.findFirst("#"+key));
}
