#ifndef PLAYERMANAGER_H
#define PLAYERMANAGER_H

#include <QObject>
#include <QStringList>


class QStandardItemModel;


class PlayerManager : public QObject
{
    Q_OBJECT
public:
    explicit PlayerManager(QObject *parent = 0);

    void checkPlayer(const QVariant data);

signals:

public slots:

private:
    QStringList                 head;
    QStandardItemModel*         s_playerModel;
};

#endif // PLAYERMANAGER_H
