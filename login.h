#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include <QtDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QEvent>
#include "tools.h"
namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

    void Init_Ctrl();
    bool connectionDataBase();

private slots:
    void on_login_btn_clicked();

    void on_register_btn_clicked();

private:
    Ui::Login *ui;
    QSqlDatabase db;

signals:
    void sendInfo(QString str);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // LOGIN_H
