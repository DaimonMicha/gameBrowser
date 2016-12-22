#ifndef PLUGIN_H
#define PLUGIN_H

#include "plugininterface.h"
#include "account.h"


class Plugin  : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "de.daimonnetwork.gameBrowser.PluginInterface" FILE "plugin.json")
    Q_INTERFACES(PluginInterface)

public:
    ~Plugin();

    QString name() const { return(QLatin1String("Plugin")); }
    QWidget* settingsWidget() const { return(new QWidget()); }
    bool isMyUrl(const QUrl &) const;

    void loadSettings(QSettings &);

    void replyFinished(QNetworkReply*);
    void loadFinished(QWebPage*);

private:
    Account *accFromCookie(const QString);
    int readDataFile(const QString file, QString& data);
    void injectHtml(QWebFrame*, Account*);

private:
    QList<Account *>      m_accounts;
};

#endif // PLUGIN_H
