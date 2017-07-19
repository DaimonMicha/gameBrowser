#ifndef BATTLEKNIGHTDOCK_H
#define BATTLEKNIGHTDOCK_H

#include <QWidget>

namespace Ui {
class BattleKnightDock;
}

class BattleKnightDock : public QWidget
{
    Q_OBJECT

public:
    explicit BattleKnightDock(QWidget *parent = 0);
    ~BattleKnightDock();

private:
    Ui::BattleKnightDock *ui;
};

#endif // BATTLEKNIGHTDOCK_H
