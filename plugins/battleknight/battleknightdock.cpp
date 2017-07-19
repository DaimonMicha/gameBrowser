#include "battleknightdock.h"
#include "ui_battleknightdock.h"

BattleKnightDock::BattleKnightDock(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BattleKnightDock)
{
    ui->setupUi(this);
}

BattleKnightDock::~BattleKnightDock()
{
    delete ui;
}
