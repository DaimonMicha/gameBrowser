#ifndef BATTLEKNIGHT_H
#define BATTLEKNIGHT_H

#include "plugininterface.h"
#include "account.h"

#include <QJsonDocument>



class jsConsole : public QObject
{
    Q_OBJECT
public:
    jsConsole(QObject *parent = 0);

public slots:
    void log(const QByteArray&);
};

class BattleKnight : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "de.daimonnetwork.gameBrowser.PluginInterface" FILE "battleknight.json")
    Q_INTERFACES(PluginInterface)

public:
    ~BattleKnight();

    QString name() const { return(QLatin1String("BattleKnight")); }
    QWidget* settingsWidget() const { return(new QWidget()); }

    void loadSettings(QSettings &);
    void saveSettings(QSettings &);
    void saveState(QSettings &);

    void replyFinished(QNetworkReply*);
    void loadFinished(QWebPage*);

private:
    Account *accFromCookie(const QString,const QUrl url = QUrl());
    void injectHtml(QWebFrame*, Account*);

private slots:
    void hasPlayer();

private:
    QList<Account *>            m_accounts;
    QJsonObject                 m_accountStates;
    jsConsole                   m_console;
};

#endif // BATTLEKNIGHT_H
