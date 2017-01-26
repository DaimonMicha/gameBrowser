#ifndef OGAME_H
#define OGAME_H

#include "plugininterface.h"
#include "account.h"


class OGame : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "de.daimonnetwork.gameBrowser.PluginInterface" FILE "ogame.json")
    Q_INTERFACES(PluginInterface)

public:
    ~OGame();

    QString name() const { return(QLatin1String("OGame")); }
    QWidget* settingsWidget() const { return(new QWidget()); }

    void loadSettings(QSettings &);
    void saveSettings(QSettings &);
    void saveState(QSettings &);

    void replyFinished(QNetworkReply*);
    void loadFinished(QWebPage*);

private:
    Account *accFromCookie(const QString);
    void injectHtml(QWebFrame*, Account*);

private:
    QList<Account *>            m_accounts;
};

#endif // OGAME_H
