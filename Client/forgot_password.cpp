#include "forgot_password.h"
#include "login.h"
#include "ui_forgot_password.h"

#include <QThread>
#include <QTimer>
#include <QDebug>
#include <QSqlError>
#include <QMessageBox>
#include "resetpassword.h"
Forgot_Password::Forgot_Password(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Forgot_Password)
{
    ui->setupUi(this);
    timeout_count=60;
    status=false;
    m_tcp=new QTcpSocket;
    Client_Thread *thread=new Client_Thread(m_tcp);
    thread->start();
    connect(thread,&Client_Thread::sendCodeSignal,this,&Forgot_Password::recvCodeSlot);
    connect(thread,&Client_Thread::sendAuthUserSignal,this,&Forgot_Password::recvCompareSlot);
    connect(thread,&Client_Thread::closeSocketSignal,this,&Forgot_Password::closeSocketSlot);


}

Forgot_Password::~Forgot_Password()
{
    delete ui;
}

QByteArray Forgot_Password::toSendData()
{
    QJsonObject *json=new QJsonObject;
    json->insert("type","cmd");
    json->insert("cmd_type","forgot_password");
    json->insert("email",ui->edit_email->text());
    json->insert("data","AUTH_CODE");
    json->insert("source_addr",m_tcp->localAddress().toString());
    json->insert("source_port",QString::number(m_tcp->localPort()));
    return convertQJsonObjectToQString(*json).toLocal8Bit();
}

QByteArray Forgot_Password::toSendAuthCode()
{
    QJsonObject *json=new QJsonObject;
    json->insert("type","cmd");
    json->insert("cmd_type","forgot_password");
    json->insert("data","RESET_PASSWORD");
    json->insert("source_addr",m_tcp->localAddress().toString());
    json->insert("source_port",QString::number(m_tcp->localPort()));
    json->insert("code",ui->edit_code->text());
    return convertQJsonObjectToQString(*json).toLocal8Bit();
}

void Forgot_Password::Init_Socket()
{
    QSettings *setting=getSettings();
    QString ServerIp=setting->value("SERVER/IP").toString();
    qint16 ServerPort=setting->value("SERVER/PORT").toInt();
    if(!status){
        m_tcp->connectToHost(ServerIp,ServerPort);
        if(m_tcp->waitForConnected(3000)){
            status=true;
            qDebug()<<"connect";
        }
        else{
            status=false;
            qDebug()<<"init";
            QMessageBox::warning(nullptr,"警告","连接服务端失败！");
        }
    }
}

bool Forgot_Password::connectionDataBase()
{
    db=QSqlDatabase::addDatabase("QMYSQL");
    QSettings *seting=getSettings();
    db.setHostName(seting->value("/Connect/IP").toString());
    db.setDatabaseName(seting->value("/Connect/DATABASE").toString());
    db.setUserName(seting->value("/Connect/USERNAME").toString());
    db.setPassword(seting->value("/Connect/PASSWORD").toString());
    delete seting;
    return db.open();
}

bool Forgot_Password::Authentication()
{
    bool checkInputFlag = checkInput();
    if(checkInputFlag){
        bool db_flag = connectionDataBase();
        if(db_flag){
            if(db.isOpen()){
                QString sql="select userId from ClientUsers where username='%1'";
                QSqlQuery query(sql.arg(ui->edit_username->text()));
                if(query.lastError().text().isEmpty()){
                    QString userId = query.value("userId").toString();
                    sql="select count(id) from Client_Info where user_email='%1'";
                    query.exec(sql.arg(ui->edit_email->text()));
                    if(query.lastError().text().isEmpty()){
                        return true;
                    }
                }
            }
        }
    }else
        QMessageBox::warning(nullptr,"警告","请检查输入！");

    return false;
}

bool Forgot_Password::checkInput()
{
    if(ui->edit_username->text().isEmpty())
        return false;
    if(ui->edit_email->text().isEmpty())
        return false;
    return true;
}

void Forgot_Password::on_btn_sendCode_clicked()
{
    if(Authentication()){
        Init_Socket();

        if(m_tcp->isWritable())
            m_tcp->write(toSendData());
    }
}

void Forgot_Password::timeout_slot()
{
    ui->btn_sendCode->setText(QString::number(--timeout_count));
    if(timeout_count==0){
        timer->stop();
        timeout_count=60;
        ui->btn_sendCode->setEnabled(true);
        ui->btn_sendCode->setText("发送验证码");

    }
}
/** 接收验证码发送结果
 * @brief Forgot_Password::recvCodeSlot
 * @param flag
 */
void Forgot_Password::recvCodeSlot(bool flag)
{
    if(flag){
        qDebug()<<"exec recvCodeSlot";
        ui->btn_sendCode->setEnabled(false);
        timer=new QTimer;
        connect(timer,&QTimer::timeout,this,&Forgot_Password::timeout_slot);
        timer->start(1000);
        QMessageBox::information(nullptr,"信息","验证码已经发送，请注意查收！");
    }
    else{
        QMessageBox::warning(nullptr,"警告","验证码发送失败！");
    }

}
/** 接收验证码比较结果
 * @brief Forgot_Password::recvCompareSlot
 * @param flag
 */
void Forgot_Password::recvCompareSlot(bool flag)
{
    qDebug()<<"recvCompareSlot"<<flag;
    //准备进入重新设置密码界面
    if(flag){
       ResetPassword *reset=new ResetPassword(ui->edit_username->text());
       reset->show();
       close();
    }
    else{
        QMessageBox::warning(nullptr,"警告","验证码输入错误！");
    }
}

void Forgot_Password::closeSocketSlot()
{
    status=false;
}

void Forgot_Password::on_btn_commit_clicked()
{
    if(checkInput()){
        if(!ui->edit_code->text().isEmpty()){
            if(m_tcp->isWritable())
                m_tcp->write(toSendAuthCode());
        }
    }
}

