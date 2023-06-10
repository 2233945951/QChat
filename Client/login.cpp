#include "login.h"
#include "ui_login.h"
#include "clientmsg.h"
#include "register.h"
#include "clientmsg.h"
#include "forgot_password.h"

#include <QDateTime>
#include <QRegExpValidator>
Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    Init_Ctrl();
}

Login::~Login()
{
    delete ui;
}

void Login::Init_Ctrl()
{
    ui->edit_username->setValidator(new QRegExpValidator(QRegExp("^[A-Za-z]+$")));
    ui->label_forgot_password->installEventFilter(this);
}

bool Login::connectionDataBase()
{
    db=QSqlDatabase::addDatabase("QMYSQL");
    QSettings *seting=getSettings();
    db.setHostName(seting->value("/Connect/IP").toString());
    db.setDatabaseName(seting->value("/Connect/DATABASE").toString());
    db.setUserName(seting->value("/Connect/USERNAME").toString());
    db.setPassword(seting->value("/Connect/PASSWORD").toString());
    return db.open();
}

void Login::on_login_btn_clicked()
{
    if(!ui->edit_username->text().isEmpty() && !ui->edit_username->text().isEmpty()){
        if(connectionDataBase()){
            QSqlQuery *query=new QSqlQuery(db);
            QString username=ui->edit_username->text();
            QString password=ui->edit_password->text();
            QString sql="select id from ClientUsers where username='%1' and password='%2'";
            query->exec(sql.arg(username,password));
            if(query->numRowsAffected()>0){
                sql="update ClientUsers set login_time='%1' where username='%2'";
                //更新上线时间
                query->exec(sql.arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"),username));

                qDebug()<<query->lastQuery();
                db.close();
                this->close();

                ClientMsg *clientMsg=new ClientMsg;
                connect(this,&Login::sendInfo,clientMsg,&ClientMsg::reciveInfo);
                emit sendInfo(username);
                clientMsg->show();

            }
            else
                QMessageBox::warning(nullptr,"Waring","用户名或密码错误");
        }
        else
            QMessageBox::warning(nullptr,"警告","连接数据库失败！");
    }
    else
        QMessageBox::warning(nullptr,"Waring","用户名或密码不允许为空");
    if(db.isOpen())
        db.close();
}



void Login::on_register_btn_clicked()
{
    this->close();
    Register reg;
    reg.exec();

}

bool Login::eventFilter(QObject *obj, QEvent *event)
{
    if(obj==ui->label_forgot_password){
        if(event->type()==QEvent::Type::MouseButtonPress){
            Forgot_Password *forgot=new Forgot_Password;
            forgot->show();
            this->close();

        }
    }
}

