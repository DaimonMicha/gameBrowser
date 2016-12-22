#include "pluginmanager.h"
#include "browserapplication.h"
#include "networkaccessmanager.h"

#include <QDir>
#include <QLibrary>
#include <QSettings>
#include <QPluginLoader>
#include <QWebFrame>

#include <QDebug>

PluginManager::PluginManager(QObject *parent) :
    QObject(parent)
{
    QSettings settings;
    settings.beginGroup(QLatin1String("PluginManager"));
    m_pluginPath = settings.value(QLatin1String("pluginsDir"), qApp->applicationDirPath() + "/plugins").toString();
    settings.endGroup();
    listPlugins();
    connect(BrowserApplication::networkAccessManager(), SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

void PluginManager::loadSettings()
{
    QSettings settings;
    //qDebug() << "PluginManager::loadSettings";
}

void PluginManager::saveSettings()
{
}

void PluginManager::listPlugins()
{
    QDir pluginDir(m_pluginPath);
    foreach(QString fileName, pluginDir.entryList(QDir::Files)) {
        if(QLibrary::isLibrary(pluginDir.absoluteFilePath(fileName))) {
            m_pluginFiles.append(pluginDir.absoluteFilePath(fileName));
        }
    }
    //qDebug() << "PluginManager::listPlugins" << m_pluginFiles;
}

void PluginManager::loadPlugin(const QString & filePath)
{
    QPluginLoader loader(filePath);
    QObject *plugin = loader.instance();
    if(plugin) {
        PluginInterface* e = qobject_cast<PluginInterface *>(plugin);
        if(e) {
            m_pluginsMap.insert(e, filePath);
            QSettings settings;
            e->loadSettings(settings);
            //ret = true;
            //QStandardItem* item = new QStandardItem(e->name());
            //item->setData(filePath,Qt::ToolTipRole);
            //m_botsItem->appendRow(item);
            //connect(e, SIGNAL(logMessage(QString)), this, SLOT(log(QString)));
            //qDebug() << "PluginManager::loadPlugin" << e->name();
        } else {
            qDebug() << "\t" << filePath << "is not an PluginInterface!";
        }
        //populateMenus(plugin);
    } else {
        qDebug() << "error loading:" << filePath << loader.errorString();
    }
}

void PluginManager::loadPlugins()
{
    foreach(QString fileName, m_pluginFiles) {
        loadPlugin(fileName);
        //qDebug() << "PluginManager::loadPlugins" << fileName;
    }
}

void PluginManager::replyFinished(QNetworkReply* reply)
{
    QUrl url = reply->url();
    QMapIterator<PluginInterface *, QString> i(m_pluginsMap);
    while(i.hasNext()) {
        i.next();
        if(i.key()->isMyUrl(url)) {
            i.key()->replyFinished(reply);
        }
    }
}

void PluginManager::loadFinished(QWebPage* page)
{
    QUrl url = page->mainFrame()->url();
    QMapIterator<PluginInterface *, QString> i(m_pluginsMap);
    while(i.hasNext()) {
        i.next();
        if(i.key()->isMyUrl(url)) {
            i.key()->loadFinished(page);
        }
    }
}
