#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>
#include <QSettings>
#include <QFile>
#include <QUrl>
#include <QNetworkReply>
#include <QWidget>
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
    virtual ~PluginInterface() {}

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
    virtual QWidget* settingsWidget() const { return(new QWidget()); }

    virtual void initPlugin() {
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
    }
    virtual void loadSettings(QSettings &) {}
    virtual void saveSettings(QSettings &) {}
    virtual void saveState(QSettings &) {}

    virtual void replyFinished(QNetworkReply *) {}
    virtual void loadFinished(QWebPage*) {}

    virtual int readDataFile(const QString file, QString& data)
    {
        QFile rf;
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
};

#define PluginInterface_iid "de.daimonnetworks.gameBrowser.PluginInterface"
Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)

#endif // PLUGININTERFACE_H
