#ifndef ACCMODULE_H
#define ACCMODULE_H

#include <QObject>


class bkAccount;
class QWebPage;
class QWebElement;

class accModule : public QObject
{
    Q_OBJECT
public:
    accModule(const QString& id, const QString& name, const QString& title = "", QObject *parent = 0);

    QString id() const { return m_id; }

protected:
    QWebElement findModule(QWebElement doc);
    QWebElement appendLine(QWebElement module);
    void addProgressBar(QWebElement module, QString id = "", QString text = "");

private:
    QWebElement cloneModule(QWebElement doc);

public slots:
    void pageLoaded(bkAccount*,QWebPage*);
    virtual void moduleReady(bkAccount*, QWebPage*) {}

private:
    QString                 m_id;
    QString                 m_name;
    QString                 m_title;
};


class modAccount : public accModule
{
    Q_OBJECT
public:
    modAccount(const QString& id, const QString& name, const QString& title = "", QObject *parent = 0) :
        accModule(id, name, title, parent) {}

public slots:
    virtual void moduleReady(bkAccount*, QWebPage*);
};


class modMissions : public accModule
{
    Q_OBJECT
public:
    modMissions(const QString& id, const QString& name, const QString& title = "", QObject *parent = 0) :
        accModule(id, name, title, parent) {}

public slots:
    virtual void moduleReady(bkAccount*,QWebPage*);
};



class modGM : public accModule
{
    Q_OBJECT
public:
    modGM(const QString& id, const QString& name, const QString& title = "", QObject *parent = 0) :
        accModule(id, name, title, parent) {}

public slots:
    virtual void moduleReady(bkAccount*,QWebPage*);
};




class modDuel : public accModule
{
    Q_OBJECT
public:
    modDuel(const QString& id, const QString& name, const QString& title = "", QObject *parent = 0) :
        accModule(id, name, title, parent) {}

public slots:
    virtual void moduleReady(bkAccount*,QWebPage*);
};

#endif // ACCMODULE_H
