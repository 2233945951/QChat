#include "resetpassword.h"
#include "ui_resetpassword.h"
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QSqlError>
#include <QSqlQuery>
#include "login.h"
#include "tools.h"
ResetPassword::ResetPassword(QString username,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResetPassword),
    m_username(username)
{
    ui->setupUi(this);
    setWindowTitle("重新设置密码");
    ui->edit_username->setText(m_username);
    if(!connectionDataBase()){
        QMessageBox::warning(nullptr,"警告","连接数据库失败");
    }
}

ResetPassword::~ResetPassword()
{
    delete ui;
}

bool ResetPassword::checkInput()
{
    if(ui->edit_password->text().isEmpty())
        return false;
    if(ui->edit_confirm_password->text().isEmpty())
        return false;
    if(ui->edit_password->text().indexOf(ui->edit_confirm_password->text())==-1)
        return false;
    return true;
}

bool ResetPassword::connectionDataBase()
{
    db=QSqlDatabase::addDatabase("QMYSQL");
    QSettings *seting=getSettings();
    db.setHostName(seting->value("/Connect/IP").toString());
    db.setDatabaseName(seting->value("/Connect/DATABASE").toString());
    db.setUserName(seting->value("/Connect/USERNAME").toString());
    db.setPassword(seting->value("/Connect/PASSWORD").toString());
    return db.open();
}

void ResetPassword::on_btn_commit_clicked()
{
    if(checkInput()){
        QString sql="update ClientUsers set password='%1' where username='%1'";
        if(db.isOpen()){
            QSqlQuery query(sql.arg(m_username));
            if(query.lastError().text().isEmpty()){
                Login *login=new Login;
                login->show();
                close();
            }else
                qDebug()<<query.lastError();
            }else
                qDebug()<<"连接数据库失败";
        }

    else{
        QMessageBox::warning(nullptr,"警告","请检查输入！");
    }
}

