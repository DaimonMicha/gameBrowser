#ifndef BKREPORTSMODULE_H
#define BKREPORTSMODULE_H
#include "bkmodule.h"

#include <QProcess>
#include <QJsonObject>
#include <QJsonArray>




class bkReportsModule : public bkModule
{
    Q_OBJECT
public:
    bkReportsModule(QJsonObject& defaults, bkAccount* account, QObject *parent = 0);

public slots:
    void restoreState(QJsonObject& state);
    void replyFinished(QNetworkReply* reply);
    void parse(QWebPage* page, QVariant titleTimer = QVariant(), QVariant progressbarEndTime = QVariant(), QVariant progressbarDuration = QVariant());
    void logic(QWebPage*);
    void action(QWebPage*);
    void view(QWebPage*) {}

protected slots:
    void startWorker();
    void workerFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void translatePlayer(const QJsonObject& player);
    bool checkPlayer(int id, QString dateTime);
    void parseDuel(QJsonObject mail);
    void checkMail(QJsonObject mail);
    void mailInbox(QNetworkReply*);

private:
    QJsonArray                      m_workPlayers;
    QProcess                        m_workerProcess;
};

#endif // BKREPORTSMODULE_H
