#ifndef BKREPORT_H
#define BKREPORT_H

#include <QObject>
#include <QJsonObject>



class bkReport : public QObject
{
    Q_OBJECT
public:
    explicit bkReport(int id, QObject *parent = 0);
    ~bkReport();

    int id() const { return report_id; }
    bool isEmpty() { return m_data.isEmpty(); }

signals:

public slots:
    void setData(const QJsonObject& data);

private:
    int             report_id;
    QJsonObject     m_data;

    QStringList     m_keys;
};

#endif // BKREPORT_H
