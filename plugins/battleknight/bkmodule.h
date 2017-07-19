#ifndef BKMODULE_H
#define BKMODULE_H

#include <QObject>
#include <QJsonObject>
#include <QJsonValue>

class QNetworkReply;
class QWebPage;

class bkModule : public QObject
{
    Q_OBJECT
public:
    explicit bkModule(QString name, QJsonObject& defaults, QObject *parent = 0);
    ~bkModule();

    QString name() const { return(m_name); }

signals:

public slots:
    void replyFinished(QNetworkReply*);
    void loadFinished(QWebPage*);
    virtual void setState(const QString& key, QJsonValue value);
    virtual void restoreState(QJsonObject& state);
    virtual QJsonObject saveState();

    virtual void parse(QWebPage*);
    virtual void logic(QWebPage*);
    virtual void view(QWebPage*);

    friend QDebug operator << (QDebug dbg, const bkModule* module);

private slots:

protected:
    QJsonObject                 m_state;

private:
    QString                     m_name;
};

#endif // BKMODULE_H
