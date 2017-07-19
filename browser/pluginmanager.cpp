#include "pluginmanager.h"
#include "browserapplication.h"
#include "browsermainwindow.h"
#include "networkaccessmanager.h"

#include <QDir>
#include <QBuffer>
#include <QLibrary>
#include <QSettings>
#include <QPluginLoader>
#include <QWebFrame>
#include <QDockWidget>

#include <QDebug>




PluginManager::PluginManager(QObject *parent) :
    QObject(parent)
{
    // Create seed for the random
    // That is needed only once on application startup
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

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
    settings.beginGroup(QLatin1String("PluginManager"));
    QMapIterator<PluginInterface *, QString> i(m_pluginsMap);
    while(i.hasNext()) {
        i.next();
        i.key()->loadSettings(settings);
    }
    settings.endGroup();
    //qDebug() << "PluginManager::loadSettings" << m_pluginsMap.count();
}

void PluginManager::saveSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("PluginManager"));
    settings.setValue(QLatin1String("pluginsDir"), m_pluginPath);

    QMapIterator<PluginInterface *, QString> i(m_pluginsMap);
    while(i.hasNext()) {
        i.next();
        i.key()->saveSettings(settings);
        //qDebug() << "PluginManager::saveSettings()" << i.key()->name();
    }

    settings.endGroup();
}

void PluginManager::saveState()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("PluginManager"));
    settings.setValue(QLatin1String("pluginsDir"), m_pluginPath);

    QMapIterator<PluginInterface *, QString> i(m_pluginsMap);
    while(i.hasNext()) {
        i.next();
        i.key()->saveState(settings);

        // get dockWidgets...
        BrowserMainWindow *browser = BrowserApplication::instance()->mainWindow();
        QList<QDockWidget *> widgets = browser->findChildren<QDockWidget *>();
        foreach(QDockWidget *dw, widgets) {
            if(dw->windowTitle() == i.key()->name()) {
                settings.beginGroup(i.key()->name());
                QByteArray data;
                //bool floating, show;
                QBuffer buffer(&data);
                QDataStream stream(&buffer);
                buffer.open(QIODevice::ReadWrite);
                stream << dw->pos()
                       << dw->saveGeometry()
                       << dw->isFloating()
                       << !dw->isHidden();
                settings.setValue(QLatin1String("dockWidget"), data);
/*
                qDebug() << "PluginManager::saveState()" << i.key()->name()
                         << !dw->isHidden() << dw->isVisible() << dw->isFloating() << dw->pos();
*/
                settings.endGroup();
            }
        }
    }

    settings.endGroup();
}

void PluginManager::pluginOptionsChanged()
{
    qWarning() << "PluginManager::pluginOptionsChanged";
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
            settings.beginGroup(QLatin1String("PluginManager"));
            //e->loadSettings(settings);
            //e->initPlugin();
            QWidget *dock = e->dockWidget();
            if(dock != Q_NULLPTR) {
                //qDebug() << "loadPlugin::settingsKey:" << e->name()+"\\dockWidget";
                QDockWidget *dw = new QDockWidget(e->name());
                dw->setObjectName(e->name());
                dw->setWidget(dock);
                BrowserMainWindow *browser = BrowserApplication::instance()->mainWindow();
                browser->addDockWidget(Qt::LeftDockWidgetArea,dw);

                settings.beginGroup(e->name());
                QByteArray d = settings.value(QLatin1String("dockWidget")).toByteArray();
                QBuffer buffer(&d);
                QDataStream stream(&buffer);
                buffer.open(QIODevice::ReadWrite);
                QByteArray data;
                QPoint pos;
                bool floating, show;
                stream >> pos;
                stream >> data;     dw->restoreGeometry(data);
                stream >> floating; dw->setFloating(floating);
                stream >> show;     dw->setVisible(show);
                //qDebug() << pos;
                dw->move(pos.x(),pos.y());
                settings.endGroup();
            }
            settings.endGroup();
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
    //qDebug() << "PluginManager::loadPlugins" << m_pluginsMap.count();
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
    page->mainFrame()->addToJavaScriptWindowObject("console",&m_console);
    QUrl url = page->mainFrame()->url();
    QMapIterator<PluginInterface *, QString> i(m_pluginsMap);
    while(i.hasNext()) {
        i.next();
        if(i.key()->isMyUrl(url)) {
            i.key()->loadFinished(page);
        }
    }
}
