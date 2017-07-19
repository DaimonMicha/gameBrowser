#include "bkmodulemanager.h"
#include "bkaccountmodule.h"

#include <QNetworkReply>
#include <QWebPage>
#include <QWebElement>
#include <QJsonDocument>

#include <QDebug>







bkModuleManager::bkModuleManager(QJsonObject& defaults, QObject *parent) :
    QObject(parent)
{
    p_account = qobject_cast<bkAccount*>(parent);
    Q_ASSERT(p_account);
    if(defaults.contains("Account")) {
        QJsonObject config = defaults.value("Account").toObject();
        QPointer<bkModule> modul = new bkAccountModule(config);
        m_modules.append(modul);
    }
    qDebug() << "bkModuleManager" << "created.";
}

bkModuleManager::~bkModuleManager()
{
    while(m_modules.count() > 0) {
        delete m_modules.takeFirst();
    }
    qDebug() << "bkModuleManager" << "destroyed.";
}

void bkModuleManager::restoreState(QJsonObject& state)
{
    foreach(QPointer<bkModule> modul, m_modules) {
        if(state.contains(modul->name())) {
            QJsonObject config = state.value(modul->name()).toObject();
            modul->restoreState(config);
        }
    }
}

QJsonObject bkModuleManager::saveState()
{
    QJsonObject state;
    foreach(QPointer<bkModule> modul, m_modules) {
        state.insert(modul->name(), modul->saveState());
    }
    //qDebug() << "bkModuleManager::saveState" << QJsonDocument(state).toJson().data();
    return state;
}

void bkModuleManager::replyFinished(QNetworkReply* reply)
{
    foreach(QPointer<bkModule> modul, m_modules) {
        if(modul) {
            modul->replyFinished(reply);
        }
    }
}

void bkModuleManager::loadFinished(QWebPage* page)
{
    foreach(QPointer<bkModule> modul, m_modules) {
        if(modul) {
            modul->loadFinished(page);
        }
    }
    foreach(QPointer<bkModule> modul, m_modules) {
        if(modul) {
            //modul->logic(page);
        }
    }
    // ToDo:
    // doing the job.
    foreach(QPointer<bkModule> modul, m_modules) {
        if(modul) {
            //modul->view(page);
        }
    }
}
