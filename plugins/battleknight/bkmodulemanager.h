#ifndef BKMODULEMANAGER_H
#define BKMODULEMANAGER_H

#include <QObject>
#include <QPointer>
#include <QJsonObject>
#include "bkaccount.h"
#include "bkmodule.h"

class QNetworkReply;
class QWebPage;
class bkAccount;

class bkModuleManager : public QObject
{
    Q_OBJECT
public:
    explicit bkModuleManager(QJsonObject& defaults, QObject *parent = 0);
    ~bkModuleManager();

signals:

public slots:
    void replyFinished(QNetworkReply*);
    void loadFinished(QWebPage*);
    void restoreState(QJsonObject& state);
    QJsonObject saveState();

private:
    bkAccount*                      p_account;
    QList<QPointer<bkModule> >      m_modules;
};

#endif // BKMODULEMANAGER_H
