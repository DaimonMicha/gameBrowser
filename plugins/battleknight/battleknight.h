#ifndef BATTLEKNIGHT_H
#define BATTLEKNIGHT_H

#include "plugininterface.h"
#include "account.h"


class BattleKnight : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "de.daimonnetwork.gameBrowser.PluginInterface" FILE "battleknight.json")
    Q_INTERFACES(PluginInterface)

public:
    ~BattleKnight();

    QString name() const { return(QLatin1String("BattleKnight")); }
    QWidget* settingsWidget() const { return(new QWidget()); }
    bool isMyUrl(const QUrl &) const;

    void initPlugin();
    void loadSettings(QSettings &);
    void saveSettings(QSettings &);

    void replyFinished(QNetworkReply*);
    void loadFinished(QWebPage*);

    int readDataFile(const QString file, QString& data);

private:
    Account *accFromCookie(const QString);
    void injectHtml(QWebFrame*, Account*);

private:
    PluginSettings              m_settings;
    QStringList                 m_excludeExtensions;
    QList<Account *>            m_accounts;
};

#endif // BATTLEKNIGHT_H
