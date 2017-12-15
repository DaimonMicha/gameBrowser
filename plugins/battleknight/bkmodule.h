#ifndef BKMODULE_H
#define BKMODULE_H

#include <QObject>
#include <QJsonObject>
#include <QJsonValue>
#include <QWebElement>
#include <QDateTime>
#include <QTimer>

class QNetworkReply;
class QWebPage;
class bkAccount;


class bkModule : public QObject
{
    Q_OBJECT
public:
    bkModule(QString name, QJsonObject& defaults, bkAccount* account, QObject *parent = 0);
    ~bkModule();

    QString name() const { return(m_name); }
    QString stateName(int key) const { return(m_stateNames.value(key)); }
    bool active() { return(state("active").toBool()); }
    bool enabled() { return(state("enable").toBool()); }
    bool hasPath(const QString& path) { return(m_includePaths.contains(path.toLower())); }
    int strength() { return(state("strength").toInt()); }

    friend QDebug operator << (QDebug dbg, const bkModule* module);

    enum modes {
        mode_unknown,
        mode_waiting,
        mode_working
    };

    enum cooldownModes {
        mode_none = 50,
    };

    enum travelModes {
        travel_bestWork,
        travel_bestTax
    };
    enum wearModes {
        wear_forGM,
        wear_forTravel,
        wear_forMission,
        wear_forDuel,
        wear_forWork,
        wear_forTournament
    };

protected:
    void addPath(const QString& path) { if(!hasPath(path)) m_includePaths.append(path.toLower()); }
    void addStateName(int key, const QString& value) { m_stateNames.insert(key, value); }
    QWebElement getModule(QWebElement doc);
    QWebElement cloneModule(QWebElement doc);
    QWebElement appendLine(QWebElement module);
    void addProgressBar(QWebElement module, QString id = "", QString text = "");
    void startActionTimer(int maxWait);
    void stopActionTimer() { m_actionTimer.stop(); setState("wait", 0); }
    void updated();
    QDateTime lastUpdate() const { return(m_lastUpdate); }

signals:
    void jobDone();
    void hasJobFor(const QString& module, const QString& job, const int mode);

public slots:
    virtual void activate() { setState("active", true); }
    virtual void deactivate() { setState("active", false); }
    virtual void job(const QString& jobName, const int& mode) { Q_UNUSED(jobName); Q_UNUSED(mode); }

    virtual void setState(const QString& key, QJsonValue value);
    virtual QJsonValue state(const QString& key);
    virtual void restoreState(QJsonObject& state);
    virtual QJsonObject saveState();

    virtual void replyFinished(QNetworkReply*);
    virtual void parse(QWebPage* page, QVariant titleTimer = QVariant(), QVariant progressbarEndTime = QVariant(), QVariant progressbarDuration = QVariant());
    virtual void logic(QWebPage*);
    virtual void view(QWebPage*);
    virtual void action(QWebPage*);

protected slots:
    virtual void adjustTimer() {}
    virtual void adjustPoints() {}
    virtual void actionTimer() {}

protected:
    QJsonObject                     m_state;
    bkAccount*                      p_account;

private:
    QString                         m_name;
    QStringList                     m_includePaths;
    QTimer                          m_actionTimer;
    QMap<int, QString>              m_stateNames;

    QDateTime                       m_lastUpdate;
};

#endif // BKMODULE_H
