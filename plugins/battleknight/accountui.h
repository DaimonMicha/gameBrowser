#ifndef ACCOUNTUI_H
#define ACCOUNTUI_H

#include <QObject>
#include <QWebElement>




class Account;
class QWebPage;
class QTimerEvent;

class accountUI : public QObject
{
    Q_OBJECT
public:
    accountUI(Account *account, QObject *parent = 0);
    ~accountUI();

    QWebElement addModule(QString moduleId, QString moduleName = QString(), QString moduleTitle = QString());
    void addStatusText(QWebElement module, QString key, QString label = QString());
    void addProfileText(QWebElement module, QString key, QString label = QString());
    void addLocationText(QWebElement module, QString key, QString tmpl = QString());

signals:

public slots:
    void inject();
    void check();
    void stopTimer();

protected:
    void timerEvent(QTimerEvent *event);

private:
    Account*                s_account;
    QWebPage*               s_page;

    int                     m_workingTimer;
    int                     m_animateTimer;
    QList<QWebElement>      m_checkerList;
    QList<QWebElement>      m_statusList;
    QList<QWebElement>      m_profileList;
};

#endif // ACCOUNTUI_H
