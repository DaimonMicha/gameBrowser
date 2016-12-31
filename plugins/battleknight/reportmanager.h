#ifndef REPORTMANAGER_H
#define REPORTMANAGER_H

#include <QObject>
#include <QVariant>
#include <QList>
#include <QJsonDocument>

class QTimer;
class QProcess;


class ReportManager : public QObject
{
    Q_OBJECT
public:
    explicit ReportManager(QObject *parent = 0);

    void checkReport(const QVariant data);

    QVariantMap lastReport(int maxCount, QString type = "*") const;

signals:
    void playerCheck(const QVariant data);

protected:
    void unserialize(const QString&);

public slots:
    void processNextReport();
    void phpReady();

private:
    QTimer*                 s_workingTimer;
    QProcess*               phpProc;
    QVariantMap             m_mailsMap;
    QString                 m_currentFightType;
    QString                 m_currentFightTime;
    QJsonDocument           m_currentDocument;

    bool                    isWorking;
    QList<QJsonDocument>    m_workList;
};

#endif // REPORTMANAGER_H
