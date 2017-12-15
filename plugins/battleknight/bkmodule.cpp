#include "bkmodule.h"
#include "bkaccount.h"

#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>




bkModule::bkModule(QString name, QJsonObject& defaults, bkAccount* account, QObject *parent) :
    QObject(parent),
    m_state(defaults),
    p_account(account),
    m_name(name)
{
    deactivate();
    addStateName(mode_unknown, tr("unknown"));
    addStateName(mode_waiting, tr("waiting"));
    addStateName(mode_working, tr("working"));

    m_lastUpdate = QDateTime::currentDateTime().addDays(-10);

    m_actionTimer.setSingleShot(true);
    connect(&m_actionTimer, SIGNAL(timeout()), this, SLOT(actionTimer()));
    //qDebug() << "bkModule::bkModule" << name << QJsonDocument(m_state).toJson().data();
}

bkModule::~bkModule()
{
}

void bkModule::setState(const QString& key, QJsonValue value)
{
    QJsonValue data = value;
    QString module, topic;
    if(key.startsWith("timer") && !key.endsWith("Duration")) {
        module = key.mid(5).toLower();
        topic = "timer";
        QDateTime now = QDateTime::currentDateTimeUtc();
        int count = value.toInt();
        if(count > 0) {
            data = (int)now.addSecs(count).toTime_t();
        } else {
            return;
        }
    } else if(key.endsWith("Duration")) {
        module = key.left(key.length() - 8).toLower();
        module = module.mid(5);
        topic = "duration";
    } else if(key.startsWith("modus")) {
        module = key.mid(5).toLower();
        topic = "state";
    } else if(key.startsWith("module")) {
        module = key.mid(6).toLower();
        topic = "module";
    } else {
        //return;
    }

    if(module.isEmpty()) {
        m_state.insert(key, value);
    } else {
        //if(m_state.contains(module)) {
            QJsonObject modul = m_state.value(module).toObject();
            //if(modul.contains(topic)) {
                //qDebug() << "    module found:" << QJsonDocument(modul).toJson().data();
                modul.insert(topic, data);
                m_state.insert(module.toLower(), modul);
            //}
        //}
    }
    //qDebug() << "--> bk"+name()+"Module::setState" << module << topic << key << value;
}

QJsonValue bkModule::state(const QString& key)
{
    QJsonValue ret;

    if(m_state.contains(key)) {
        ret = m_state.value(key);
        //qDebug() << "bk"+name()+"Module::state" << key << ret;
        return ret;
    }

    QString module, topic;
    if(key.startsWith("timer") && (!key.endsWith("Duration"))) {
        module = key.mid(5).toLower();
        topic = "timer";
    } else if(key.endsWith("Duration")) {
        module = key.left(key.length() - 8);
        module = module.mid(5).toLower();
        topic = "duration";
    } else if(key.startsWith("modus")) {
        module = key.mid(5).toLower();
        topic = "state";
    }

    if(m_state.contains(module)) {
        QJsonObject modul = m_state.value(module).toObject();
        if(modul.contains(topic)) {
            ret = modul.value(topic);
        }
    }

    if(!ret.isNull()) {
        if(key.startsWith("timer") && (!key.endsWith("Duration"))) {
            QDateTime now = QDateTime::currentDateTimeUtc();
            QDateTime eta = QDateTime::fromTime_t(ret.toInt());
            if(eta > now) {
                ret = now.secsTo(eta);
            } else {
                // rounds up?
                ret = 0;
            }
        }
    }

    //qDebug() << "bk"+name()+"Module::state" << module << topic << key << ret;
    return ret;
}

void bkModule::restoreState(QJsonObject& state)
{
    foreach(QString key, state.keys()) {
        //if(m_state.contains(key)) {
            m_state.insert(key, state.value(key));
        //}
    }
    //qDebug() << "--> bk" + name() + "Module::restoreState" << this;
}

QJsonObject bkModule::saveState()
{
    return m_state;
}

void bkModule::replyFinished(QNetworkReply*)
{
}
/*
void bkModule::loadFinished(QWebPage* page)
{
    parse(page);
    //logic(page);
    //view(page);
}
*/
void bkModule::parse(QWebPage*, QVariant, QVariant, QVariant)
{
}

void bkModule::logic(QWebPage*)
{
}

void bkModule::action(QWebPage*)
{
}

void bkModule::view(QWebPage* page)
{
    QWebElement moduleElement = getModule(page->mainFrame()->documentElement());
    QWebElement checkbox = moduleElement.findFirst("#enable" + name());
    if(checkbox.isNull()) return;
    if(enabled()) checkbox.setAttribute("checked","checked");
    checkbox.evaluateJavaScript("this.addEventListener('click', function(){ account.toggle(this.id, this.checked); });");
}

QWebElement bkModule::getModule(QWebElement doc)
{
    QWebElement mElement = doc.findFirst("#km" + name());
    if(mElement.isNull()) mElement = cloneModule(doc);
    if(active() && !mElement.hasClass("kmActiveTopic")) {
        mElement.addClass("kmActiveTopic");
        //qDebug() << "bkModule::getModule" << name() << mElement.classes() << active();
    }
    return mElement;
}

QWebElement bkModule::cloneModule(QWebElement doc)
{
    QWebElement module;
    QWebElement target = doc.findFirst("#accountPlugin");
    QWebElement tmpl = doc.findFirst("#ModulTemplate");
    module = tmpl.clone();
    module.setAttribute("id", "km"+name());
    target.appendInside(module);
    QString tip = state("tooltip").toString();
    if(tip.isEmpty()) tip = state("title").toString();
    // checkbox
    tmpl = module.findFirst("#enableModule");
    tmpl.setAttribute("id", "enable"+name());
    tmpl.setAttribute("title", tip);
    // label
    tmpl = module.findFirst("label");
    tmpl.setAttribute("for", "enable"+name());
    tmpl.setPlainText(state("title").toString());
    tmpl.setAttribute("title", tip);
    module.removeClass("nodisplay");
    return module;
}

QWebElement bkModule::appendLine(QWebElement module)
{
    QWebElement target = module.findFirst("tbody");
    target.appendInside("<tr><td id=\"_appendLine_\" colspan=\"2\" align=\"center\"></td></tr>");
    QWebElement insert = target.findFirst("#_appendLine_");
    insert.removeAttribute("id");
    return insert;
}

void bkModule::addProgressBar(QWebElement module, QString id, QString text)
{
    if(id.isEmpty()) id = name();
    else if(id != name()) id = name()+id;
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

    //qDebug() << "bkModule::addProgressBar" << id;
}

void bkModule::startActionTimer(int maxWait)
{
    if(m_actionTimer.isActive()) return;
    int minWait = 123;
    int wait = qrand() % (maxWait) + minWait;
    m_actionTimer.start(wait);
    setState("wait", wait);
}

void bkModule::updated()
{
    m_lastUpdate = QDateTime::currentDateTime();
}

QDebug operator << (QDebug dbg, const bkModule* module)
{
    dbg << QJsonDocument(module->m_state).toJson().data();
    return dbg;
}
