#ifndef BKITEM_H
#define BKITEM_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>



class bkItem : public QObject
{
    Q_OBJECT
public:
    explicit bkItem(int id, QObject *parent = 0);
    ~bkItem();

    int id() const { return item_id; }
    bool contains(const QString& key);
    QVariant value(const QString& key);
    QByteArray toJson() const {
        return QJsonDocument(m_data).toJson();
    }

    friend QDebug operator << (QDebug dbg, const bkItem* item);

signals:

public slots:
    void setData(const QJsonObject& data);

private:
    int             item_id;
    QJsonObject     m_data;

    QStringList     m_keys;
};

#endif // BKITEM_H
