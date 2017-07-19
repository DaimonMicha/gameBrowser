#ifndef FLEETDIALOG_H
#define FLEETDIALOG_H

#include <QDialog>

namespace Ui {
class fleetDialog;
}

class fleetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit fleetDialog(QWidget *parent = 0);
    ~fleetDialog();

private:
    Ui::fleetDialog *ui;
};

#endif // FLEETDIALOG_H
