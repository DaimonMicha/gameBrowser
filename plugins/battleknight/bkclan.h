#ifndef BKCLAN_H
#define BKCLAN_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>



class bkClan : public QObject
{
    Q_OBJECT
public:
    explicit bkClan(int id, QObject *parent = 0);
    ~bkClan();

    int id() const { return clan_id; }
    QByteArray toJson() const {
        return QJsonDocument(m_data).toJson();
    }

signals:

public slots:
    void setData(const QJsonObject& data);

private:
    int             clan_id;
    QJsonObject     m_data;

    QStringList     m_keys;
};

#endif // BKCLAN_H
