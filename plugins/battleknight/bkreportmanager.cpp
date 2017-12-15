#include "bkreportmanager.h"

#include <QDebug>



bkReportManager::bkReportManager(QObject *parent) :
    QObject(parent)
{
    //qDebug() << "bkReportManager created.";
}

bkReportManager::~bkReportManager()
{
    int rc = m_reportMap.count();
    while(!m_reportMap.isEmpty()) {
        bkReport* report = m_reportMap.take(m_reportMap.lastKey());
        delete report;
    }
    qDebug() << "bkReportManager and" << rc << "Report(s) destroyed.";
}

bkReport* bkReportManager::report(const int id)
{
    if(m_reportMap.contains(id)) {
        return m_reportMap.value(id);
    }
    bkReport* report = new bkReport(id);
    m_reportMap.insert(id, report);
    return report;
}
