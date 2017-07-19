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
    jsConsole(QObject *parent = 0) :
        QObject(parent)
    {
    }

public slots:
    void log(const QVariant& data)
    {
        qWarning() << "-> jsConsole::log:" << data.toByteArray().constData();
    }
    void info(const QVariant& data)
    {
        qWarning() << "-> jsConsole::info:" << data.toByteArray().constData();
    }
    void warn(const QVariant& data)
    {
        qWarning() << "-> jsConsole::warn:" << data.toByteArray().constData();
    }
    void debug(const QVariant& data)
    {
        qWarning() << "-> jsConsole::debug:" << data.toByteArray().constData();
    }
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
