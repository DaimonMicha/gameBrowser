#ifndef OGAMEDOCK_H
#define OGAMEDOCK_H

#include <QWidget>
#include <QJsonObject>



namespace Ui {
    class OGameDock;
}

class QStandardItemModel;



class OGameDock : public QWidget
{
    Q_OBJECT

public:
    explicit OGameDock(QWidget *parent = 0);
    ~OGameDock();

    void setConstants(QJsonObject& constants);
    void updatePlanets(const QJsonObject&);
    void updateFleets(const QJsonObject&);

protected:
    int planetRow(int pid);
    int fleetRow(int pid);
    int fleetCargo(QJsonObject&);
    int fleetCapacity(QJsonObject&);
    void timerEvent(QTimerEvent *event);

private slots:
    void fleetContextRequest(const QPoint&);

private:
    Ui::OGameDock*          ui;
    QStandardItemModel*     s_planets;
    QStandardItemModel*     s_fleets;

    int                     m_resourceTimer;
    int                     m_timeStamp;
    bool                    m_canUpdate;

    QJsonObject             m_constants;
};

#endif // OGAMEDOCK_H
