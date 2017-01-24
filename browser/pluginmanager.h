#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QStringList>
#include <QVariant>
#include "plugininterface.h"

class jsConsole : public QObject
{
    Q_OBJECT
public:
    jsConsole(QObject *parent = 0);

public slots:
    void log(const QVariant&);
    void info(const QVariant&);
    void warn(const QVariant&);
    void debug(const QVariant&);
};


class PluginManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginManager(QObject *parent = 0);

    void loadSettings();
    void saveSettings();
    void saveState();

    void loadPlugins();

protected:
    void listPlugins();
    void loadPlugin(const QString &);

signals:

public slots:
    void replyFinished(QNetworkReply*);
    void loadFinished(QWebPage*);

    void pluginOptionsChanged();

private:
    QString                             m_pluginPath;
    QStringList                         m_pluginFiles;

    QMap<PluginInterface *, QString>    m_pluginsMap;
    jsConsole                           m_console;
};

#endif // PLUGINMANAGER_H
