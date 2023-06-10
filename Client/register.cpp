#include "login.h"
#include "register.h"
#include "ui_register.h"

#include <QMessageBox>
#include <QRegExpValidator>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
Register::Register(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);
    Init_Ctrl();
}

Register::~Register()
{
    delete ui;
}

void Register::Init_Ctrl()
{
    ui->edit_username->setValidator(new QRegExpValidator(QRegExp("^[A-Za-z]+$")));
    ui->edit_email->setValidator(new QRegExpValidator(QRegExp("^[A-Za-z0-9]+([_\\.][A-Za-z0-9]+)*@([A-Za-z0-9\\-]+\\.)+[A-Za-z]{2,6}$")));
    ui->edit_phone->setValidator(new QRegExpValidator(QRegExp("^1([358][0-9]|4[579]|66|7[0135678]|9[89])[0-9]{8}$")));
}

bool Register::connectionDataBase()
{
    db=QSqlDatabase::addDatabase("QMYSQL");
    QSettings *seting=getSettings();
    db.setHostName(seting->value("/Connect/IP").toString());
    db.setDatabaseName(seting->value("/Connect/DATABASE").toString());
    db.setUserName(seting->value("/Connect/USERNAME").toString());
    db.setPassword(seting->value("/Connect/PASSWORD").toString());
    return db.open();
}

bool Register::checkInput()
{
    if(ui->edit_username->text().isEmpty()){
        QMessageBox::warning(nullptr,"warning","用户名不能为空");
        return false;
    }
    else if(ui->edit_email->text().isEmpty()){
        QMessageBox::warning(nullptr,"warning","电子邮箱不能为空");
        return false;
    }
    else if(ui->edit_phone->text().isEmpty()){
        QMessageBox::warning(nullptr,"warning","手机号码不能为空");
        return false;
    }
    else if(ui->edit_password->text().isEmpty()){
        QMessageBox::warning(nullptr,"warning","密码不能为空");
        return false;
    }
    else if(ui->edit_confirmPassword->text().isEmpty()){
        QMessageBox::warning(nullptr,"warning","重复密码不能为空");
        return false;
    }
    else if(ui->edit_password->text().length()!=ui->edit_confirmPassword->text().length() ||
            ui->edit_password->text().indexOf(ui->edit_confirmPassword->text())<-1){
        return false;
    }
    return true;
}

void Register::on_btn_submit_clicked()
{
    if(checkInput()){
        if(connectionDataBase()){
            QSqlQuery *query=new QSqlQuery(db);
            QString sql="select id from ClientUsers where username='%1'";
            QString username=ui->edit_username->text();
            QString password=ui->edit_password->text();
            query->exec(sql.arg(username));
            if(query->numRowsAffected()>0){

                QMessageBox::warning(nullptr,"warning","用户名已存在，请重新输入");
            }else{
                qsrand(QDateTime::currentSecsSinceEpoch());
                int random_value=qrand()+10000000%99999999;

                sql="insert into ClientUsers(username,password,register_time,userId) values('%1','%2','%3','%4')";
                query->exec(sql.arg(username,password,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(random_value));
                //执行插入语句没出错
                if(query->lastError().text().isEmpty()){
                    sql="insert into Client_Info(userId,user_phone,user_email) values('%1','%2','%3')";
                    QString cno=QString::number(random_value);
                    QString c_phone=ui->edit_phone->text();
                    QString c_email=ui->edit_email->text();
                    query->exec(sql.arg(cno,c_phone,c_email));
                    if(query->lastError().text().isEmpty()){
                        if(QMessageBox::information(nullptr,"infomation","注册成功，点击确定跳转至登录窗口")==QMessageBox::Ok){
                            this->close();
                            Login login;
                            login.exec();
                        }
                        else{
                            QMessageBox::warning(nullptr,"warning","注册失败");
                        }
                    }else{
                        qDebug()<<query->lastError();
                    }
                }
            }
        }
        else
            QMessageBox::warning(nullptr,"警告","连接数据库失败！");

    }
    else
        QMessageBox::warning(nullptr,"警告","请将信息填写完整！");
}



