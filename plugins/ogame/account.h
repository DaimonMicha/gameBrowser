#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QObject>
#include <QWebPage>
#include <QVariant>



class Account : public QObject
{
    Q_OBJECT
public:
    Account(const QString cookie, QObject *parent = 0);

    Q_INVOKABLE QString cookieValue() const { return(m_cookieValue); }
    Q_INVOKABLE bool isActive(const QString option = "enableAccount") const {
        if(m_botOptions.contains(option)) return(m_botOptions.value(option));
        return(false);
    }

signals:

public slots:
    void toggle(const QString option = "account", const bool on = false);
    void loadFinished(QWebPage*);
    void replyFinished(QNetworkReply*);

private:
    QMap<QString, bool>     m_botOptions;
    QString                 m_cookieValue;
    QNetworkAccessManager*  s_networkManager;
};

#endif // ACCOUNT_H
