#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QStringList>
#include "plugininterface.h"

class PluginManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginManager(QObject *parent = 0);

    void loadSettings();
    void saveSettings();

    void loadPlugins();

protected:
    void listPlugins();
    void loadPlugin(const QString &);

signals:

public slots:
    void replyFinished(QNetworkReply*);
    void loadFinished(QWebPage*);

private:
    QString             m_pluginPath;
    QStringList         m_pluginFiles;

    QMap<PluginInterface *, QString> m_pluginsMap;
};

#endif // PLUGINMANAGER_H
