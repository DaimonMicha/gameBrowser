#ifndef BKMODULEMANAGER_H
#define BKMODULEMANAGER_H

#include <QObject>
#include <QPointer>
#include <QJsonObject>
#include "bkaccount.h"
#include "bkmodule.h"

class QNetworkReply;
class QWebPage;

class bkModuleManager : public QObject
{
    Q_OBJECT
public:
    explicit bkModuleManager(QJsonObject& defaults, QObject *parent = 0);
    ~bkModuleManager();

protected:
    bkModule* addModule(const QString& module);

signals:

public slots:
    void restoreState(QJsonObject& state);
    QJsonObject saveState();
    void toggle(const QString module, const bool soll);
    void replyFinished(QNetworkReply*);
    void loadFinished(QWebPage*);

    QVariant state(const QString module, const QString key);

protected slots:
    void setState(const QString& key, QJsonValue value);
    QJsonValue state(const QString& key);

    void logic(QWebPage*);
    void view(QWebPage*);

    void moduleHasJobFor(const QString& module, const QString& job, const int& mode);
    void moduleIsReady();

private:
    bkAccount*                      p_account;
    QJsonObject                     m_settings;
    QStringList                     m_reloadPaths;
    QList<QPointer<bkModule> >      m_modules;
    QPointer<bkModule>              p_accountModul = NULL;
    QPointer<bkModule>              p_activeModul = NULL;

    QJsonObject                     m_state;
};

#endif // BKMODULEMANAGER_H
