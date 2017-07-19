#ifndef BATTLEKNIGHT_H
#define BATTLEKNIGHT_H

#include "plugininterface.h"
#include "accmodule.h"


#include <QJsonDocument>



class BattleKnightDock;
class bkWorld;
class bkAccount;


class BattleKnight : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "de.daimonnetwork.gameBrowser.PluginInterface" FILE "battleknight.json")
    Q_INTERFACES(PluginInterface)

public:
    BattleKnight();
    ~BattleKnight();

    QString name() const { return(QLatin1String("BattleKnight")); }
    //QWidget* settingsWidget() const { return(new QWidget()); }

    void loadSettings(QSettings &);
    void saveSettings(QSettings &);
    void saveState(QSettings &);

    void replyFinished(QNetworkReply*);
    void loadFinished(QWebPage*);

private:
    QString getCookie(const QUrl&);
    bkAccount *findAccount(const QUrl&);
    void injectHtml(QWebFrame*);

private slots:
    void updateMP();

private:
    QList<bkWorld *>            m_worldList;
    QList<bkAccount *>          m_accountList;

    QList<QPointer<accModule> > m_modules;

    QList<QPointer<QWebPage> >  m_webPages;

    QJsonObject                 m_moduleDefaults;
    QJsonObject                 m_accountStates;
    BattleKnightDock*           m_browserDock;
};

#endif // BATTLEKNIGHT_H
