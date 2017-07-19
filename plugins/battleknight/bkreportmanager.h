#ifndef BKREPORTMANAGER_H
#define BKREPORTMANAGER_H

#include <QObject>
#include <QMap>

#include "bkreport.h"




class bkReportManager : public QObject
{
    Q_OBJECT
public:
    explicit bkReportManager(QObject *parent = 0);
    ~bkReportManager();

    bkReport* report(const int id);

signals:

public slots:

private:
    QMap<int, bkReport*>        m_reportMap;
};

#endif // BKREPORTMANAGER_H
