#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QDialog>
#include <QTableView>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRelationalTableModel>
#include "tools.h"
#include <QSqlRelationalDelegate>
#include "customdelegate.h"
namespace Ui {
class UserManager;
}

class UserManager : public QDialog
{
    Q_OBJECT

public:
    explicit UserManager(QWidget *parent = nullptr);
    ~UserManager();
    void Init();
    bool connectionDataBase();
private:
    Ui::UserManager *ui;
    QTableView *tableView;
    QSqlDatabase db;
    QSqlRelationalTableModel *model;
};

#endif // USERMANAGER_H
