#include "accmodule.h"
#include "bkaccount.h"

#include <QWebPage>
#include <QWebElement>

#include <QDebug>




accModule::accModule(const QString& id, const QString& name, const QString& title, QObject *parent) :
    QObject(parent),
    m_id(id),
    m_name(name),
    m_title(title)
{
    if(m_title.isEmpty()) m_title = name;
}

QWebElement accModule::findModule(QWebElement doc)
{
    QWebElement mElement = doc.findFirst("#km" + m_id);
    if(mElement.isNull()) mElement = cloneModule(doc);
    return mElement;
}

QWebElement accModule::cloneModule(QWebElement doc)
{
    QWebElement module;
    QWebElement target = doc.findFirst("#accountPlugin");
    QWebElement tmpl = doc.findFirst("#ModulTemplate");
    module = tmpl.clone();
    module.setAttribute("id", "km"+m_id);
    target.appendInside(module);
    // checkbox
    tmpl = module.findFirst("#enableModule");
    tmpl.setAttribute("id", "enable"+m_id);
    tmpl.setAttribute("title", m_title);
    // label
    tmpl = module.findFirst("label");
    tmpl.setAttribute("for", "enable"+m_id);
    tmpl.setPlainText(m_name);
    tmpl.setAttribute("title", m_title);
    module.removeClass("nodisplay");
    return module;
}

QWebElement accModule::appendLine(QWebElement module)
{
    QWebElement target = module.findFirst("tbody");
    target.appendInside("<tr><td id=\"_appendLine_\" colspan=\"2\" align=\"center\"></td></tr>");
    QWebElement insert = target.findFirst("#_appendLine_");
    insert.removeAttribute("id");
    return insert;
}

void accModule::addProgressBar(QWebElement module, QString id, QString text)
{
    if(id.isEmpty()) id = m_id;
    QWebElement bar = module.findFirst("div#progressbarText" + id);
    if(!bar.isNull()) return;
    if(!text.isEmpty()) text.append(": ");
    QWebElement target = appendLine(module);
    //  style=\"border: 1px solid yellow;\"
    target.appendInside("<div id=\"progressbarText" + id + "\" style=\"position: relative; width: 100%; height: 18px;\">"
                        "<div align=\"left\" id=\"timeMeter" + id + "\" class=\"kmProgressMeter\" style=\"width: 0%; height: 16px; position: absolute; background-color: rgba(255, 0, 0, 0.35);\"></div>"
                        "<div class=\"km_progressText\" style=\"display: inline;\">" + text + "</div><span></span>"
                        "</div>");

    QWebElement progressDiv = target.findFirst("#progressbarText" + id);
    progressDiv.setAttribute("km_timer", "on");
    QString script;
    script = "kmUpdater.addProgress('" + id + "');\n";
    if(module.webFrame() == NULL) module.evaluateJavaScript(script);
    else module.webFrame()->evaluateJavaScript(script);
}

void accModule::pageLoaded(bkAccount* account, QWebPage* page)
{
    QWebElement document = page->mainFrame()->documentElement();
    QWebElement moduleElement = findModule(document);
    QWebElement checkbox = moduleElement.findFirst("#enable" + m_id);
    if(!checkbox.isNull())
        checkbox.evaluateJavaScript("this.addEventListener('click', function(){ account.toggle(this.id, this.checked); });");

    if(!account->enabled(m_id))
        return;

    checkbox.setAttribute("checked","checked");
    QVariant result = account->status("timer" + m_id);
    if(result.isValid()) addProgressBar(moduleElement);
    moduleReady(account, page);
}





void modAccount::moduleReady(bkAccount* account, QWebPage* page)
{
    QWebElement document = page->mainFrame()->documentElement();
    QWebElement moduleElement = findModule(document);
    QWebElement knightName = moduleElement.findFirst("#kmKnightName");
    knightName.setPlainText(account->player()->value("knight_name").toString());
    knightName.setAttribute("align", "right");
    QVariant result = account->status("timerLogic");
    if(result.isValid()) {
        int modus = account->status("modusLogic").toInt();
        QString m = "unknown";
        switch(modus) {
            case w_nothing:
                m = "relax";
                break;
            case w_call_working:
                m = "call working";
                break;
            case w_go_working:
                m = "go working";
                break;
            case w_get_paying:
                m = "get paying";
                break;
            case w_call_gm:
                m = "call gm";
                break;
            case w_reload_gm:
                m = "reload gm";
                break;
            case w_call_duel:
                m = "call duell";
                break;
            case w_check_proposal:
                m = "check proposal";
                break;
            case w_next_proposal:
                m = "next proposal";
                break;
            case w_go_compare:
                m = "go compare";
                break;
            case w_go_duel:
                m = "go duel";
                break;
            default:
                break;
        }

        addProgressBar(moduleElement, "Logic", m);
        QWebElement progressMeter = moduleElement.findFirst("#timeMeterLogic");
        progressMeter.setStyleProperty("background-color", "rgba(255, 255, 0, 0.30)");
        moduleElement.findFirst("#timeMeterLogic").setAttribute("km_direction", "down");
    }

    result = account->status("timerCooldown");
    if(result.isValid()) {
        int modus = account->status("modusCooldown").toInt();
        QString m = "unknown";
        switch(modus) {
            case cd_work:
                m = "Arbeiten";
                break;
            case cd_duel:
                m = "Duell";
                break;
            case cd_fight:
                m = "Mission";
                break;
            case cd_travel:
                m = "Reise";
                break;
            default:
                break;
        }
        addProgressBar(moduleElement, "Cooldown", m);

        //QWebElement progressMeter = moduleElement.findFirst("#timeMeterCooldown");
        //progressMeter.setStyleProperty("height", "16px");
        //progressMeter.setStyleProperty("background-color", "rgba(0, 255, 0, 0.30)");
    }

    result = account->status("timerKarma");
    if(result.isValid()) {
        addProgressBar(moduleElement, "Karma", "Karma");
        QWebElement progressMeter = moduleElement.findFirst("#timeMeterKarma");
        progressMeter.setStyleProperty("background-color", "rgba(0, 255, 0, 0.30)");
    }
}




void modMissions::moduleReady(bkAccount* account,QWebPage* page)
{
    QWebElement document = page->mainFrame()->documentElement();
    QWebElement moduleElement = findModule(document);
    QWebElement text = moduleElement.findFirst("#kmMissionPoints");
    if(!text.isNull()) return;
    QVariant result = account->status("location");
    if(result.isValid()) {
        text = appendLine(moduleElement);
        QString content = account->world()->locationId(result.toString()).value("title").toString();
        result = account->status("missionPoints");
        if(result.isValid()) {
            content.append(QString(" (<span>%1</span>)").arg(result.toInt()));
        }
        text.appendInside(content);
        text.setAttribute("id", "kmMissionPoints");
        text.setAttribute("align", "right");
        QString script = "var MPUpdate = function() {\n"
                         "  var text = parseInt(account.status('missionPoints'));\n"
                         "  document.id('kmMissionPoints').getElement('span').set('text', text);\n"
                         "}\n"
                         "MPUpdate();\n";
        page->mainFrame()->evaluateJavaScript(script);
        //qDebug() << "modMissions::moduleReady(timerMP)" << script.toLocal8Bit().data();
    }
}



void modGM::moduleReady(bkAccount* account,QWebPage* page)
{
    QWebElement document = page->mainFrame()->documentElement();
    QWebElement moduleElement = findModule(document);
    QWebElement text = moduleElement.findFirst("#kmGMPoints");
    if(!text.isNull()) return;
    QVariant result = account->status("groupMissionPoints");
    if(result.isValid()) {
        QWebElement text = appendLine(moduleElement);
        QString content = QString("(<span>%1</span>)").arg(result.toString());
        result = account->status("modusGM");
        if(result.toInt() == gm_in_group) content.prepend("in group ");
        text.appendInside(content);
        text.setAttribute("id", "kmGMPoints");
        text.setAttribute("align", "right");
        QString script = "var GMUpdate = function() {\n"
                         "  var text = parseInt(account.status('groupMissionPoints'));\n"
                         "  document.id('kmGMPoints').getElement('span').set('text', text);\n"
                         "}\n"
                         "GMUpdate();\n";
        page->mainFrame()->evaluateJavaScript(script);
    }
}



void modDuel::moduleReady(bkAccount* account,QWebPage* page)
{
    QWebElement document = page->mainFrame()->documentElement();
    QWebElement moduleElement = findModule(document);
    //bkWorld* world = account->world();
/*
    QVariant result = account->status("groupMissionPoints");
    if(result.isValid()) {
        QWebElement text = appendLine(moduleElement);
        QString content = QString("(%1)").arg(result.toString());
        text.setPlainText(content);
        text.setAttribute("align", "right");
    }
*/
}
