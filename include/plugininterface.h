#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

//#include "../browser/browserapplication.h"
#include <QtPlugin>
#include <QSettings>
#include <QFile>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QWidget>
#include <QToolBox>
#include <QWebPage>

class PluginSettings
{
public:
    bool                        enabled;
    QString                     templatePath;
    QStringList                 urlPatterns;
};

class PluginInterface
{
public:
    PluginInterface() :
        s_settingsWidget(Q_NULLPTR),
        s_dockWidget(Q_NULLPTR),
        s_networkManager(Q_NULLPTR)
    {
        Q_INIT_RESOURCE(data);
        m_excludeExtensions
                << "js"
                << "mp3"
                << "ogg"
                << "css"
                << "cur"
                << "ico"
                << "gif"
                << "png"
                << "jpg"
                << "woff"
                << "swf"
                   ;
        //s_networkManager = reinterpret_cast<QNetworkAccessManager *>(BrowserApplication::networkAccessManager());
    }
    virtual ~PluginInterface() {
        Q_CLEANUP_RESOURCE(data);
    }

    virtual QString name() const { return(QLatin1String("PluginInterface")); }
    virtual bool isMyUrl(const QUrl &url) const {
        QString host = url.host();
        foreach(QString pattern, m_pluginSettings.urlPatterns) {
            QRegExp rx(pattern);
            rx.setPatternSyntax(QRegExp::Wildcard);
            if(rx.exactMatch(host)) return(true);
            if(host.endsWith(pattern)) return(true);
        }
        return(false);
    }
    virtual QWidget* settingsWidget() const { return(s_settingsWidget); }
    virtual QWidget* dockWidget() const { return(s_dockWidget); }

    //virtual void initPlugin() = 0;
    virtual void loadSettings(QSettings &) = 0;
    virtual void saveSettings(QSettings &) = 0;
    virtual void saveState(QSettings &) = 0;

    virtual void replyFinished(QNetworkReply *) {}
    virtual void loadFinished(QWebPage*) {}

    virtual int readDataFile(const QString file, QString& data)
    {
        QFile rf;
        if(!m_pluginSettings.templatePath.endsWith("/")) m_pluginSettings.templatePath.append("/");
        rf.setFileName(m_pluginSettings.templatePath + file);
        if(!rf.open(QIODevice::ReadOnly)) {
            rf.setFileName(":/"+name().toLower()+"/" + file);
            if(!rf.open(QIODevice::ReadOnly)) return(-1);
        }
        QByteArray bytes = rf.readAll();
        rf.close();
        data.truncate(0);
        data.append(bytes);
        return(data.length());
    }

protected:
    PluginSettings              m_pluginSettings;
    QStringList                 m_excludeExtensions;
    QWidget*                    s_settingsWidget;
    QWidget*                    s_dockWidget;
    QNetworkAccessManager*      s_networkManager;
};

#define PluginInterface_iid "de.daimonnetworks.gameBrowser.PluginInterface"
Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)

#endif // PLUGININTERFACE_H
