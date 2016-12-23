#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>
#include <QSettings>
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
    virtual bool isMyUrl(const QUrl &) const { return(false); }
    virtual QWidget* settingsWidget() const { return(new QWidget()); }

    virtual void initPlugin() {}
    virtual void loadSettings(QSettings &) {}
    virtual void saveSettings(QSettings &) {}

    virtual void replyFinished(QNetworkReply *) {}
    virtual void loadFinished(QWebPage*) {}
};

#define PluginInterface_iid "de.daimonnetworks.gameBrowser.PluginInterface"
Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)

#endif // PLUGININTERFACE_H
