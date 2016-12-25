#ifndef ITEMMANAGER_H
#define ITEMMANAGER_H

#include <QObject>

class ItemManager : public QObject
{
    Q_OBJECT
public:
    explicit ItemManager(QObject *parent = 0);
    ~ItemManager();

    void checkItem(const QVariant data);

signals:

public slots:

};

#endif // ITEMMANAGER_H
