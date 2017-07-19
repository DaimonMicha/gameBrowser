#ifndef BKLOGIC_H
#define BKLOGIC_H

#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QJsonObject>

class bkAccount;
class QWebPage;

struct workerState
{
    workerState(QString name) :
        m_name(name)
    {
    }

    QString     m_name;
};

enum workerStates {
    w_nothing, // 0
    w_call_working,
    w_go_working,
    w_get_paying,
    w_call_gm,
    w_reload_gm, // 5
    w_call_duel,
    w_check_proposal,
    w_next_proposal,
    w_go_compare,
    w_go_duel // 10
};


class bkLogic : public QObject
{
    Q_OBJECT
public:
    explicit bkLogic(QObject *parent = 0);

    QVariant status(const QString key);

signals:

public slots:
    void loadFinished(QWebPage*);
    void checkProposal();

private slots:
    void startWorker(int min, int max);
    void workerTimer();
    void checkWorking();
    void checkGM();
    void goWorking();
    void calculateStrength();

private:
    bkAccount*                  p_account;
    workerStates                m_workerState;
    QTimer                      t_workerTimer;
    QUrl                        m_baseUrl;

    QJsonObject                 m_state;
};

#endif // BKLOGIC_H
