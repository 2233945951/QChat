#include "clientmsg.h"
#include "ui_clientmsg.h"

#include <QDateTime>
#include <QDir>
#include <QHostInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QUdpSocket>
#include <QSqlQuery>
#include <QSqlError>
#include "tools.h"
ClientMsg::ClientMsg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClientMsg)
{
    ui->setupUi(this);

    Init_fontComboBox();
    socket=new QTcpSocket();
    socketState=false;

    thread=new Client_Thread(m_file,socket,this);
    Init_Signal_Slots();
    ui->tableWidget_onlineList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QSettings *setting=getSettings();
    ui->edit_ip->setText(setting->value("SERVER/IP").toString());
    ui->edit_port->setText(setting->value("SERVER/PORT").toString());
    delete  setting;

}

ClientMsg::~ClientMsg()
{
    delete ui;
}

QJsonObject ClientMsg::toSendMsgData()
{
    //要发送：主机名，IP地址，传送的数据，用户名
    QJsonObject *jsonObject=new QJsonObject;
    jsonObject->insert("type",QJsonValue("msg"));
    jsonObject->insert("source_addr",QJsonValue(getIpRangeStr().at(0)));
    jsonObject->insert("data",QJsonValue(ui->edit_send->text()));
    jsonObject->insert("userName",QJsonValue(ui->edit_username->text()));
    return *jsonObject;
}

void ClientMsg::paddingToOnlineTable(QJsonObject json)
{
    ui->tableWidget_onlineList->clearContents();
    //如果端口号不一致，则认为是两个客户端
    if(jsonList.isEmpty())
        jsonList.append(json);
    else{
        for(const auto &obj:qAsConst(jsonList)){
            if(obj.value("source_port").toString().indexOf(json.value("source_port").toString())==-1)
                jsonList.append(json);
        }
    }
    for(int i(0);i<jsonList.count();i++){
        QString str=jsonList.at(i).value("source_port").toString();
        if(!str.isEmpty()){
            ui->tableWidget_onlineList->setItem(i,0,new QTableWidgetItem(jsonList.at(i).value("userName").toString()));
            ui->tableWidget_onlineList->setItem(i,1,new QTableWidgetItem(jsonList.at(i).value("source_addr").toString()+":"+str));
        }
    }
}


/** 初始化选择字体下拉列表
 * @brief ClientMsg::Init_fontComboBox
 */
void ClientMsg::Init_fontComboBox()
{

    for(int i(1);i<=72;i++){
        ui->comboBox_fontSize->addItem(QString::number(i));
    }
    ui->comboBox_fontSize->setCurrentText("18");
}

void ClientMsg::Init_Signal_Slots()
{
    connect(thread,&Client_Thread::sendMsg,this,&ClientMsg::recvMsg);
    connect(thread,&Client_Thread::closeSocketSignal,this,&ClientMsg::closeSocketSlot);
    connect(thread,&Client_Thread::sendFileContentSignal,this,&ClientMsg::sendFileContentSlot);
    connect(thread,&Client_Thread::recvFileContentSignal,this,&ClientMsg::recvFileContentSlot);
    connect(thread,&Client_Thread::sendFileHeader,this,&ClientMsg::recvFileHeader);
    connect(thread,&Client_Thread::finish_file_transfer,this,&ClientMsg::finish_file_transfer);
    thread->start();
}

/** 待发送的文件头
 * @brief ClientMsg::toSendFileHeader
 * @param filename 文件名
 * @param fileSize 文件大小
 * @return
 */
QJsonObject ClientMsg::toSendFileHeader(QString filename, int fileSize)
{
    QJsonObject *json=new QJsonObject;
    json->insert("scope","public");
    json->insert("type","file");
    json->insert("sender_addr",socket->localAddress().toString());
    json->insert("sender_port",QString::number(socket->localPort()));
    json->insert("fileType","fileHeader");
    json->insert("fileName",filename);
    json->insert("fileSize",fileSize);
    return *json;
}

/** 待发送的应答数据包
 * @brief ClientMsg::toSendReply
 * @return
 */
QJsonObject ClientMsg::toSendReply()
{
    QJsonObject *json=new QJsonObject;
    json->insert("scope","public");
    json->insert("type","reply");
    json->insert("replyContent","ACK");
    return *json;
}

QByteArray ClientMsg::getFileContent(QFile *file)
{
    file->open(QFile::OpenModeFlag::ReadWrite);
    QByteArray localReadAll = file->readAll();
    file->close();
    return localReadAll;
}

bool ClientMsg::connectionDataBase()
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

QString ClientMsg::text_color_black(QString string)
{

    QString str="<font color=black style=\"font-size:%1;font-family:%2;font-weight:%3;font-style:%4;text-decoration:%5;\">%6</font> <font color=black> </font>";
    QString fontSize=ui->comboBox_fontSize->currentText();
    QString fontFamily=ui->fontComboBox->currentText();
    QString fontWeight="";
    QString fontStyle="";
    QString underline="";

    if(text_isBold())
        fontWeight="bold";
    else
        fontWeight="normal";

    if(text_isItalic())
        fontStyle="italic";
    else
        fontStyle="normal";

    if(text_isUnderline())
        underline="underline";
    else
        underline="none";
    return str.arg(fontSize+"px",fontFamily,fontWeight,fontStyle,underline,string);
}

bool ClientMsg::text_isBold()
{
    return isBold;
}

bool ClientMsg::text_isItalic()
{
    return isItalic;
}

bool ClientMsg::text_isUnderline()
{
    return isUnderline;
}





QJsonObject ClientMsg::sendPrivateChatData(QStringList data)
{
    QJsonObject *json=new QJsonObject;
    json->insert("userName",data.at(0));
    json->insert("des_addr",data.at(1));
    json->insert("des_port",data.at(2));
    json->insert("source_addr",QJsonValue(socket->localAddress().toString()));
    json->insert("source_port",QJsonValue(QString::number(socket->localPort())));
    return *json;
}

void ClientMsg::disconnSlot()
{
    ui->text_chat->append(text_color_red(QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"))));
    ui->text_chat->append("和服务器断开连接");
    ui->btn_connect->setText("连接服务器");
}

void ClientMsg::reciveInfo(QString str)
{

    ui->edit_username->setText(str);
}

void ClientMsg::recvClosePrivateWindow()
{
    qDebug()<<"触发recvClosePrivateWindow";
    Init_Signal_Slots();
}

void ClientMsg::bytesWrittenSlot()
{
        if(!m_file->isOpen())
            m_file->open(QIODevice::ReadOnly);
        QByteArray arr=m_file->read(40960);
        if(arr.isEmpty()){
            m_file->close();
            disconnect(socket,&QTcpSocket::bytesWritten,this,&ClientMsg::bytesWrittenSlot);
            return;
        }
        m_sendFileSize+=socket->write(arr);
        qDebug()<<m_sendFileSize<<QFileInfo(*m_file).size();
}

void ClientMsg::recvMsg(QJsonObject json)
{
    QString strTitle = "["+json.value("userName").toString()+"]"+QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));   //本地GBK转Unicode 解决乱码
    QString strData=json.value("data").toString();

    ui->text_chat->append(text_color_blue(strTitle));
    ui->text_chat->append(text_color_black(strData));

    //不把自己和服务端的ip显示到右侧
    if(json.value("type").toString().isEmpty() || json.value("userName").toString().indexOf("Server")<0){
        paddingToOnlineTable(json);
    }
}

void ClientMsg::closeSocketSlot()
{
    ui->text_chat->append(text_color_red(QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"))));
    ui->text_chat->append("和服务器断开连接");
    ui->btn_connect->setText("连接服务器");
}

void ClientMsg::sendFileContentSlot()
{
    connect(socket,&QTcpSocket::bytesWritten,this,&ClientMsg::bytesWrittenSlot);
    bytesWrittenSlot();



}
/**
 * @brief ClientMsg::recvFileContentSlot 获取每一段传输的文件大小
 * @param size
 */
void ClientMsg::recvFileContentSlot(int size)
{

    if(!isRecv_processbar_display){
        recv_file_processbar=new QProgressBar(this);
        recv_file_processbar->setFixedSize(200,30);
        recv_file_processbar->move((width()-recv_file_processbar->width())/2,
                                   (height()-recv_file_processbar->height())/2);
        recv_file_processbar->setMaximum(m_fileSize);
        isRecv_processbar_display=true;
        recv_file_processbar->setWindowModality(Qt::ApplicationModal);

        recv_file_processbar->show();
    }
    recv_file_processbar->setValue(size);
}

void ClientMsg::recvFileHeader(QJsonObject json)
{
    m_fileName=json.value("fileName").toString();
    m_fileSize=json.value("fileSize").toInt();
}

void ClientMsg::finish_file_transfer()
{
    recv_file_processbar->close();
    delete recv_file_processbar;
    isRecv_processbar_display=false;
}
/** 发送
 * @brief ClientMsg::on_btn_send_clicked
 */
void ClientMsg::on_btn_send_clicked()
{
    QByteArray str = convertQJsonObjectToQString(toSendMsgData()).toLocal8Bit();
    if(socket->isOpen() && socket->isValid()){
        socket->write(str);
        QString strTitle="["+ui->edit_username->text()+"] "+QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
        QString strData=ui->edit_send->text();
        ui->text_chat->append(text_color_blue(strTitle));
        ui->text_chat->append(text_color_black(strData));
        ui->edit_send->clear();
    }else{
        QMessageBox::warning(nullptr,"warning","发送失败！");
    }
}

/** 连接服务端
 * @brief ClientMsg::on_btn_connect_clicked
 */
void ClientMsg::on_btn_connect_clicked()
{
    QString ServerIp=ui->edit_ip->text();
    QString ServerPort=ui->edit_port->text();
    qInfo()<<getIpRangeStr();
    //判断socket是否处于未连接状态
    if(!socketState){
        socket->connectToHost(ServerIp,ServerPort.toUInt());
        if(socket->waitForConnected(3000)){
            ui->btn_connect->setText("断开连接");
            socketState=true;
        }else{
            QMessageBox::information(nullptr,"infomation","建立连接失败！"+socket->errorString());
        }
    }
    else{
        socketState=false;
        socket->close();
        socket->disconnectFromHost();
        ui->btn_connect->setText("连接服务器");

    }
}


/** 在线列表点击进入私聊
 * @brief ClientMsg::on_tableWidget_onlineList_itemDoubleClicked
 * @param item
 */
void ClientMsg::on_tableWidget_onlineList_itemDoubleClicked(QTableWidgetItem *item)
{
    QStringList *data=new QStringList;
    if(!item->text().isEmpty()){
        *data<<ui->tableWidget_onlineList->item(item->row(),0)->text()//用户名
            <<ui->tableWidget_onlineList->item(item->row(),1)->text().split(":").at(0)//ip地址
           <<ui->tableWidget_onlineList->item(item->row(),1)->text().split(":").at(1);//端口号
    }
    socket->disconnect();
    thread->disconnect();
    PrivateChat privateChat(socket);
    connect(this,&ClientMsg::sendData,&privateChat,&PrivateChat::recvData);
    connect(&privateChat,&PrivateChat::closeWindow,this,&ClientMsg::recvClosePrivateWindow);
    emit sendData(sendPrivateChatData(*data));
    privateChat.exec();
    delete data;
}




void ClientMsg::on_btn_sendFile_clicked()
{
    QFileDialog *fileDialog=new QFileDialog(this);
    fileDialog->setFilter(QDir::Filters::enum_type::Files);
    QString str_fileName=fileDialog->getOpenFileName();
    if(!str_fileName.isEmpty()){
        m_file=new QFile(str_fileName);
        QString dataHeader = convertQJsonObjectToQString(toSendFileHeader(QFileInfo(str_fileName).fileName(),QFileInfo(str_fileName).size()));
        if(socket->isOpen() && socket->isValid()){
            socket->write(dataHeader.toLocal8Bit());
        }
        m_sendFileSize=0;
    }
}


void ClientMsg::on_btn_italic_clicked()
{
    //如果按下设置斜体按钮
    if(isItalic){
        isItalic=false;
        ui->btn_italic->setDown(isItalic);
    }else{
        isItalic=true;
        ui->btn_italic->setDown(isItalic);
    }
}

void ClientMsg::on_btn_underLine_clicked()
{
    //如果按下设置下划线按钮
    if(isUnderline){
        isUnderline=false;
        ui->btn_underLine->setDown(isUnderline);
    }else{
        isUnderline=true;
        ui->btn_underLine->setDown(isUnderline);
    }
}


void ClientMsg::on_btn_bold_clicked()
{
    //如果按下设置加粗按钮
    if(isBold){
        isBold=false;
        ui->btn_bold->setDown(isBold);
    }else{
        isBold=true;
        ui->btn_bold->setDown(isBold);
    }
}


void ClientMsg::on_btn_clearRecord_clicked()
{
    ui->text_chat->clear();
}

/** 手动保存聊天记录
 * @brief ClientMsg::on_btn_saveRecord_clicked
 */
void ClientMsg::on_btn_saveRecord_clicked()
{
    QTextDocument *localDocument = ui->text_chat->document();
    qDebug()<<localDocument->toPlainText();

    QFile *file=new QFile(QString::number(QDateTime::currentMSecsSinceEpoch()));
    file->open(QIODevice::Text|QIODevice::WriteOnly);
    file->write(ui->text_chat->document()->toPlainText().toLocal8Bit());
    file->close();
}

/** 个人信息修改
 * @brief ClientMsg::on_btn_modifyInfo_clicked
 */
void ClientMsg::on_btn_modifyInfo_clicked()
{
    connectionDataBase();
    if(db.isOpen()){
        QString sql="SELECT a.userId,a.username,b.user_phone,b.user_email,"
                    "a.login_time,a.register_time from ClientUsers a join "
                    "Client_Info b on a.userId=b.userId WHERE username='%1'";
        QSqlQuery query(sql.arg(ui->edit_username->text()));
        while (query.next()) {
            modifyWindow=new QDialog;
            modifyWindow->setWindowModality(Qt::WindowModality::ApplicationModal);
            modifyWindow->setFixedSize(400,200);
            modifyWindow->setWindowTitle("修改个人信息");

            QFormLayout *f_layout=new QFormLayout;
            QLineEdit *edit_id=new QLineEdit(query.value("userId").toString());
            QLineEdit *edit_phone=new QLineEdit(query.value("user_phone").toString());
            QLineEdit *edit_email=new QLineEdit(query.value("user_email").toString());
            QLineEdit *edit_username=new QLineEdit(query.value("username").toString());
            edit_id->setEnabled(false);

            f_layout->addRow(new QLabel("用户ID "),edit_id);
            f_layout->addRow(new QLabel("用户名 "),edit_username);
            f_layout->addRow(new QLabel("手机号 "),edit_phone);
            f_layout->addRow(new QLabel("电子邮箱 "),edit_email);



            QDateTime register_dt(query.value("register_time").toDateTime());
            QLineEdit *register_time=new QLineEdit(register_dt.toString("yyyy-MM-dd hh:mm:ss"));
            register_time->setEnabled(false);
            f_layout->addRow(new QLabel("注册时间 "),register_time);

            QDateTime login_dt(query.value("login_time").toDateTime());
            QLineEdit *login_time=new QLineEdit(login_dt.toString("yyyy-MM-dd hh:mm:ss"));
            login_time->setEnabled(false);
            f_layout->addRow(new QLabel("登录时间 "),login_time);

            QPushButton *btn=new QPushButton("确定");

            QVBoxLayout *v_layout=new QVBoxLayout;
            v_layout->addItem(f_layout);
            v_layout->addWidget(btn);

            modifyWindow->setLayout(v_layout);
            modifyWindow->show();

            connect(btn,&QPushButton::clicked,this,[=](){
                connectionDataBase();
                QString sql="update ClientUsers set username='%1' where userId='%2'";
                QSqlQuery query(sql.arg(edit_username->text(),edit_id->text()),db);
                qDebug()<<query.lastQuery();
                //如果执行更新用户名没有出错
                if(query.lastError().text().isEmpty()){
                    //开始更新用户信息
                    sql="update Client_Info set user_phone='%1',user_email='%2' where userId='%3'";
                    query.exec(sql.arg(edit_phone->text(),edit_email->text(),edit_id->text()));
                    qDebug()<<query.lastQuery();
                    if(query.lastError().text().isEmpty()){
                        ui->edit_username->setText(edit_username->text());
                        modifyWindow->close();
                        delete modifyWindow;
                    }
                    else{
                        QMessageBox::warning(nullptr,"警告","修改个人信息失败！");
                    }
                }
                else{
                    qDebug()<<"update error::"<<query.lastError().text();
                }
            });

        }
        db.close();
    }

}

