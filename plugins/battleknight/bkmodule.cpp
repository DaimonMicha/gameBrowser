#include "bkmodule.h"

#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>




bkModule::bkModule(QString name, QJsonObject& defaults, QObject *parent) :
    QObject(parent),
    m_state(defaults),
    m_name(name)
{
    //qDebug() << "bkModule::bkModule" << name << QJsonDocument(m_state).toJson().data();
}

bkModule::~bkModule()
{
}

void bkModule::setState(const QString& key, QJsonValue value)
{
    QString module, topic;
    if(key.startsWith("timer") && !key.endsWith("Duration")) {
        module = key.mid(5);
        topic = "timer";
    } else if(key.endsWith("Duration")) {
        module = key.left(key.length() - 8);
        module = module.mid(5);
        topic = "duration";
    } else if(key.startsWith("modus")) {
        module = key.mid(5);
        topic = "state";
    }

    qDebug() << "--> bkModule::setState" << module << topic << key;

    if(m_state.contains(key)) {
        m_state.insert(key, value);
    }
}

void bkModule::restoreState(QJsonObject& state)
{
    foreach(QString key, state.keys()) {
        if(m_state.contains(key)) {
            m_state.insert(key, state.value(key));
            qDebug() << "bkModule::restoreState" << name() << QJsonDocument(state).toJson().data();
        }
    }
}

QJsonObject bkModule::saveState()
{
    return m_state;
}

void bkModule::replyFinished(QNetworkReply*)
{
}

void bkModule::loadFinished(QWebPage* page)
{
    parse(page);
    logic(page);
    view(page);
}

void bkModule::parse(QWebPage*)
{
}

void bkModule::logic(QWebPage*)
{
}

void bkModule::view(QWebPage*)
{
}

QDebug operator << (QDebug dbg, const bkModule* module)
{
    dbg.space() << QJsonDocument(module->m_state).toJson().data();
    return dbg;
}
