#include "fleetdialog.h"
#include "ui_fleetdialog.h"

fleetDialog::fleetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::fleetDialog)
{
    ui->setupUi(this);
}

fleetDialog::~fleetDialog()
{
    delete ui;
}
