#include "ogamedock.h"
#include "ui_ogamedock.h"
#include "fleetdialog.h"

#include <QTimerEvent>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QMenu>

#include <math.h>

#include <QDebug>


OGameDock::OGameDock(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OGameDock),
    s_planets(new QStandardItemModel()),
    s_fleets(new QStandardItemModel()),
    m_canUpdate(true)
{
    ui->setupUi(this);
    QStringList head;
    head << "Planet" << "Koords" << "Metall" << "Kristall" << "Deuterium" << "Energie" << "Größe";
    s_planets->setHorizontalHeaderLabels(head);
    s_planets->setProperty("canUpdate", true);
    ui->planetsView->setModel(s_planets);
    head.clear();
    head << "Mission" << "Ankunft" << "Ziel" << "Cargo" << "Platz";
    s_fleets->setHorizontalHeaderLabels(head);
    s_fleets->setProperty("canUpdate", true);
    ui->fleetsView->setModel(s_fleets);

    connect(ui->fleetsView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(fleetContextRequest(QPoint)));

    m_timeStamp = QDateTime::currentDateTime().toTime_t();
    m_resourceTimer = startTimer(500);
}

void OGameDock::fleetContextRequest(const QPoint& point)
{
    QModelIndex idx = ui->fleetsView->indexAt(point);
    QMenu cm;
    cm.addAction("Details");
    QAction *ret = cm.exec(ui->fleetsView->mapToGlobal(point));
    if(ret) {
        if(ret->text() == "Details") {
            fleetDialog dlg;
            int what = dlg.exec();
            if(what == 1) {
                qDebug() << "fleetContextRequest" << point << s_fleets->item(idx.row())->data(Qt::UserRole + 1).toString() << what;
            }
        }
    }
}

OGameDock::~OGameDock()
{
    delete(ui);
    delete(s_planets);
    delete(s_fleets);
}

void OGameDock::setConstants(QJsonObject& constants)
{
    m_constants = constants;
}

void OGameDock::updateFleets(const QJsonObject& fleets)
{
    for(int r = s_fleets->rowCount();r > 0; --r) {
        if(!fleets.contains(s_fleets->item(r - 1, 0)->data(Qt::UserRole + 1).toString())) {
            s_fleets->removeRow(r - 1);
        }
    }

    QStandardItem* item;
    foreach(QJsonValue p, fleets) {
        QJsonObject fleet = p.toObject();
        int row = fleetRow(fleet.value("id").toInt());
        item = s_fleets->item(row, 0);
        item->setData(fleet.value("type").toInt(),Qt::UserRole + 2);
        item->setText(QString("%1").arg(fleet.value("type").toInt()));
        foreach(QJsonValue mission, m_constants.value("missions").toArray()) {
            if(mission.toObject().value("id").toInt() == fleet.value("type").toInt()) {
                item->setText(QString("%1").arg(mission.toObject().value("name").toString()));
                break;
            }
        }
        qint64 ts = fleet.value("arrival").toInt();
        QDateTime arrival = QDateTime::fromMSecsSinceEpoch(ts * 1000);
        s_fleets->item(row, 1)->setText(arrival.toString("hh:mm:ss"));
        s_fleets->item(row, 2)->setText(fleet.value("destPlanet").toString());
        s_fleets->item(row, 2)->setData(fleet.value("destCoords").toString(), Qt::ToolTipRole);
        s_fleets->item(row, 3)->setText(QString("%1").arg(fleetCargo(fleet)));
        s_fleets->item(row, 4)->setText(QString("%1").arg(fleetCapacity(fleet) - fleetCargo(fleet)));
    }

    QJsonDocument debug(fleets);
    ui->debugTextEdit->setPlainText(debug.toJson(QJsonDocument::Indented));

}

int OGameDock::fleetCargo(QJsonObject& fleet)
{
    int ret = 0;
    QJsonArray cargo = fleet.value("cargo").toArray();
    foreach(QJsonValue row, cargo) {
        QJsonObject c = row.toObject();
        ret += c.value("amount").toInt();
    }
    return(ret);
}

int OGameDock::fleetCapacity(QJsonObject& fleet)
{
    int ret = 0;
    QJsonArray techs = m_constants.value("techs").toArray();
    QJsonArray ships = fleet.value("ships").toArray();
    foreach(QJsonValue row, ships) {
        foreach(QJsonValue tech, techs) {
            if(tech.toObject().value("id").toInt() == row.toObject().value("id").toInt()) {
                QJsonObject data = tech.toObject().value("data").toObject();
                ret += data.value("capacity").toInt() * row.toObject().value("amount").toInt();
                break;
            }
        }
    }
    return(ret);
}

int OGameDock::fleetRow(int pid)
{
    int ret = 0;
    QModelIndexList rows = s_fleets->match(s_fleets->index(0,0),Qt::UserRole + 1,pid,1,Qt::MatchExactly | Qt::MatchRecursive);
    if(rows.size() > 0) {
        ret = rows.at(0).row();
    } else {
        QList<QStandardItem *> rowList;
        QStandardItem* item = new QStandardItem();
        item->setData(pid,Qt::UserRole + 1);
        rowList.append(item);
        for(int c = 1;c < s_fleets->columnCount(); ++c) {
            item = new QStandardItem();
            if(c > 1) item->setTextAlignment(Qt::AlignRight);
            rowList.append(item);
        }
        s_fleets->appendRow(rowList);
        ret = item->row();
    }
    return(ret);
}

void OGameDock::timerEvent(QTimerEvent *event)
{
    if(event->timerId() != m_resourceTimer) return;
    if(!s_planets->property("canUpdate").toBool()) return;

    int now = QDateTime::currentDateTime().toTime_t();
    int delta = now - m_timeStamp;
    if(delta == 0) return;

    for(int r = s_planets->rowCount();r > 0; --r) {
        float value, prod, cap;
        QStandardItem* item;

        item  = s_planets->item(r-1, 2); // metal
        cap   = item->data(Qt::UserRole + 4).toFloat();
        value = item->data(Qt::UserRole + 3).toFloat();
        prod  = item->data(Qt::UserRole + 2).toFloat();
        if(prod > 0 && value < cap) {
            value = value + prod/3600 * delta;
            item->setData(value,Qt::UserRole + 3);
            item->setText(QString("%1").arg((int)nearbyint(value)));
        }

        item  = s_planets->item(r-1, 3); // crystal
        cap   = item->data(Qt::UserRole + 4).toFloat();
        value = item->data(Qt::UserRole + 3).toFloat();
        prod  = item->data(Qt::UserRole + 2).toFloat();
        if(prod > 0 && value < cap) {
            value = value + prod/3600 * delta;
            item->setData(value,Qt::UserRole + 3);
            item->setText(QString("%1").arg((int)nearbyint(value)));
        }

        item  = s_planets->item(r-1, 4); // deuterium
        cap   = item->data(Qt::UserRole + 4).toFloat();
        value = item->data(Qt::UserRole + 3).toFloat();
        prod  = item->data(Qt::UserRole + 2).toFloat();
        if(prod > 0 && value < cap) {
            value = value + prod/3600 * delta;
            item->setData(value,Qt::UserRole + 3);
            item->setText(QString("%1").arg((int)nearbyint(value)));
        }
    }
    m_timeStamp = now;
}

int OGameDock::planetRow(int pid)
{
    int ret = 0;
    QModelIndexList rows = s_planets->match(s_planets->index(0,0),Qt::UserRole + 1,pid,1,Qt::MatchExactly | Qt::MatchRecursive);
    if(rows.size() > 0) {
        ret = rows.at(0).row();
    } else {
        QList<QStandardItem *> rowList;
        QStandardItem* item = new QStandardItem();
        item->setData(pid,Qt::UserRole + 1);
        rowList.append(item);
        for(int c = 1;c < s_planets->columnCount(); ++c) {
            item = new QStandardItem();
            if(c > 1) item->setTextAlignment(Qt::AlignRight);
            rowList.append(item);
        }
        s_planets->appendRow(rowList);
        ret = item->row();
    }
    return(ret);
}

void OGameDock::updatePlanets(const QJsonObject& planets)
{
    s_planets->setProperty("canUpdate", false);

    for(int r = s_planets->rowCount();r > 0; --r) {
    }

    foreach(QJsonValue p, planets) {
        QJsonObject planet = p.toObject();
        int row = planetRow(planet.value("id").toInt());
        s_planets->item(row, 0)->setText(planet.value("name").toString());
        s_planets->item(row, 1)->setText(planet.value("coords").toString());
        s_planets->item(row, 6)->setText(QString("%1").arg(planet.value("size").toInt()));

        if(planet.contains("resources")) {
            QJsonObject res = planet.value("resources").toObject();
            s_planets->item(row, 2)->setText(res.value("metal").toString());
            s_planets->item(row, 2)->setData(res.value("metal").toString().toFloat(),Qt::UserRole + 3);
            s_planets->item(row, 3)->setText(res.value("crystal").toString());
            s_planets->item(row, 3)->setData(res.value("crystal").toString().toFloat(),Qt::UserRole + 3);
            s_planets->item(row, 4)->setText(res.value("deuterium").toString());
            s_planets->item(row, 4)->setData(res.value("deuterium").toString().toFloat(),Qt::UserRole + 3);
            s_planets->item(row, 5)->setText(res.value("energy").toString());
        }
        if(planet.contains("production")) {
            QJsonObject res = planet.value("production").toObject();
            s_planets->item(row, 2)->setData(res.value("metal").toString(),Qt::UserRole + 2);
            s_planets->item(row, 2)->setData("+ "+res.value("metal").toString(),Qt::ToolTipRole);
            s_planets->item(row, 3)->setData(res.value("crystal").toString(),Qt::UserRole + 2);
            s_planets->item(row, 3)->setData("+ "+res.value("crystal").toString(),Qt::ToolTipRole);
            s_planets->item(row, 4)->setData(res.value("deuterium").toString(),Qt::UserRole + 2);
            s_planets->item(row, 4)->setData("+ "+res.value("deuterium").toString(),Qt::ToolTipRole);
            s_planets->item(row, 5)->setData(res.value("energy").toString(),Qt::UserRole + 2);
        }
        if(planet.contains("capacity")) {
            QJsonObject res = planet.value("capacity").toObject();
            s_planets->item(row, 2)->setData(res.value("metal").toString(),Qt::UserRole + 4);
            //s_planets->item(row, 2)->setData("+ "+res.value("metal").toString(),Qt::ToolTipRole);
            s_planets->item(row, 3)->setData(res.value("crystal").toString(),Qt::UserRole + 4);
            //s_planets->item(row, 3)->setData("+ "+res.value("crystal").toString(),Qt::ToolTipRole);
            s_planets->item(row, 4)->setData(res.value("deuterium").toString(),Qt::UserRole + 4);
            //s_planets->item(row, 4)->setData("+ "+res.value("deuterium").toString(),Qt::ToolTipRole);
        }
    }

    QJsonDocument debug(planets);
    ui->debugTextEdit->appendPlainText(debug.toJson(QJsonDocument::Indented));

    //s_planets->sortRole()

    s_planets->setProperty("canUpdate", true);
}
