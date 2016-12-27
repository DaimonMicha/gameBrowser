#ifndef REPORTMANAGER_H
#define REPORTMANAGER_H

#include <QObject>
#include <QVariant>

class QProcess;


class ReportManager : public QObject
{
    Q_OBJECT
public:
    explicit ReportManager(QObject *parent = 0);

    void checkReport(const QVariant data);

signals:
    void playerCheck(const QVariant data);

protected:
    void unserialize(const QString&);

public slots:
    void phpReady();

private:
    QProcess*       phpProc;
    QVariantMap     m_mailsMap;
    QString         m_currentFightTime;
};

#endif // REPORTMANAGER_H
