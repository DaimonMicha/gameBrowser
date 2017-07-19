#ifndef OGAME_H
#define OGAME_H

#include "plugininterface.h"
#include "account.h"

class OGameDock;

class OGame : public QObject, public PluginInterface
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "de.daimonnetwork.gameBrowser.PluginInterface" FILE "ogame.json")
    Q_INTERFACES(PluginInterface)

public:
    //OGame() : PluginInterface() { s_dockWidget = new QToolBox(); }
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
    QByteArray lastServerLogin(QUrl&);

private slots:
    void hasPlayer();

private:
    QList<Account *>            m_accounts;
    OGameDock*                  m_browserDock;
};

#endif // OGAME_H
