
#include "server.h"
#include "ui_server.h"
#include <QDateTime>
#include <QFileDialog>
#include <QJsonDocument>
#include <QMessageBox>
#include "welcome.h"
#include "usermanager.h"
Server::Server(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Server)
{
    ui->setupUi(this);


    Init();



}

Server::~Server()
{
    delete ui;
}

void Server::Init()
{
    //初始化数据库
    //initConection();
    //初始化IP列表
    initComboIpEnum();
    //初始化字体大小列表
    initComboBoxFontsize();
    m_Server=new QTcpServer;
    //有新客户端连接时
    connect(m_Server,&QTcpServer::newConnection,this,&Server::newConnectionSlot);
    //设置上下文菜单策略
    ui->listWidget_onlineList->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    //设置右键菜单
    connect(ui->listWidget_onlineList,&QWidget::customContextMenuRequested,
            this,&Server::customContextMenuRequestedSlot);
    connect(ui->listWidget_onlineList,&QListWidget::itemChanged,this,[](){
        qDebug()<<"changed";
    });
    heartbreat_confirmation();
}

void Server::initComboIpEnum()
{
    QList<QHostAddress> AddressesList = QNetworkInterface::allAddresses();
    for (auto i :AddressesList ) {
        if(QAbstractSocket::IPv4Protocol==i.protocol())
            ui->combo_ipList->addItem(i.toString());
    }
    ui->combo_ipList->setCurrentIndex(1);

}

void Server::initComboBoxFontsize()
{
    for(int i(1);i<=72;i++){
        ui->comboBox_fontSize->addItem(QString::number(i));
    }
    ui->comboBox_fontSize->setCurrentText("18");
}

void Server::initConection()
{
    if(!connectionDatabase())
        qDebug()<<"connection error";
}

void Server::paddingOnlineList()
{
    qDebug()<<m_ClientList.count();
    ui->listWidget_onlineList->clear();
    for (int i(0);i<m_ClientList.count() ;i++ ) {
        //        if(QString::number(m_ClientList.at(i)->peerPort()).indexOf("0")>-1){
        ui->listWidget_onlineList->addItem(m_ClientList.at(i)->peerAddress().toString()+":"+QString::number(m_ClientList.at(i)->peerPort()));
        //        }
    }
}

//QJsonObject Server::convertQStringToJsonObject(QString str)
//{
//    QJsonDocument jsonDocument=QJsonDocument::fromJson(str.toLocal8Bit());
//    if(!jsonDocument.isNull()){
//        return jsonDocument.object();
//    }
//    return *new QJsonObject;
//}

//QString Server::convertQJsonObjectToQString(QJsonObject json)
//{
//    return QString(QJsonDocument(json).toJson());
//}

QJsonObject Server::toSendData()
{
    //要发送：主机名，IP地址，传送的数据，用户名
    QJsonObject *jsonObject=new QJsonObject;
    jsonObject->insert("hostName",QJsonValue(QHostInfo::localHostName()));
    jsonObject->insert("addr",QJsonValue(ui->combo_ipList->currentText()));
    jsonObject->insert("data",QJsonValue(ui->edit_send->Plain));
    jsonObject->insert("userName",QJsonValue("Server"));
    jsonObject->insert("source_port",QJsonValue(ui->edit_localPort->text()));
    jsonObject->insert("type","msg");
    return *jsonObject;
}

QJsonObject Server::toSendFileHeader(QString filename, int fileSize)
{
    QJsonObject *json=new QJsonObject;
    json->insert("scope","public");
    json->insert("type","file");
    json->insert("fileType","fileHeader");
    json->insert("fileName",filename);
    json->insert("fileSize",fileSize);
    return *json;
}

QByteArray Server::getFileContent(QFile *file)
{
    file->open(QFile::OpenModeFlag::ReadWrite);
    QByteArray localReadAll = file->readAll();
    file->close();
    return localReadAll;
}

QByteArray Server::toSendCmd()
{
    QJsonObject *json=new QJsonObject;
    json->insert("type","cmd");
    json->insert("data","transfer");
    return convertQJsonObjectToQString(*json).toLocal8Bit();
}

QByteArray Server::toSendUntransferData()
{
    QJsonObject* json=new QJsonObject;
    json->insert("type","cmd");
    json->insert("data","untransfer");
    return convertQJsonObjectToQString(*json).toLocal8Bit();
}

bool Server::connectionDatabase()
{
    db=QSqlDatabase::addDatabase("QMYSQL");
    QSettings *seting=getSetting();
    db.setHostName(seting->value("/connect/IP").toString());
    db.setDatabaseName(seting->value("/connect/DATABASE_SERVER").toString());
    db.setUserName(seting->value("/connect/USERNAME").toString());
    db.setPassword(seting->value("/connect/PASSWORD").toString());
    return db.open();
}



void Server::forward(QTcpSocket *current, QJsonObject jsonObject)
{
    for(int index = 0;index < m_ClientList.count();index ++)
    {

        QTcpSocket* temp = m_ClientList.at(index);
        if(current == temp){
            continue;
        }
        if(jsonObject.value("userName").toString().indexOf("Server")>-1)
            continue;
        if(temp->isWritable())
        {
            qDebug()<<"exec transit";
            jsonObject.insert("source_addr",current->peerAddress().toString());
            jsonObject.insert("source_port",QString::number(current->peerPort()));
            temp->write(convertQJsonObjectToQString(jsonObject).toLocal8Bit());
        }


    }
}

void Server::forward(QByteArray arr)
{
    if(!m_ClientList.isEmpty()){
        for (int i(0);i<m_ClientList.count() ;i++ ) {
            if(m_ClientList[i]->isWritable()){
                m_ClientList[i]->write(arr);
            }
        }


    }
}

void Server::heartbreat_confirmation()
{
    QTimer *timer=new QTimer;
    connect(timer,&QTimer::timeout,this,&Server::heartbeat_confirmation_slot);
    timer->start(10000);
}

void Server::heartbeat_confirmation_slot(){
    qDebug()<<"exec heartbeat_confirmation_slot";
    qDebug()<<"set "<<m_ClientSet.count();
    forward(toSendHeartbeatData());

//    if(m_ClientSet.count()>0){
//        foreach(const auto &i,m_ClientSet){
//            i->disconnectFromHost();
//        }
//        m_ClientSet.clear();
//    }


}

//QString Server::text_color_blue(QString str)
//{
//    return "<font size=5 color=blue>"+str+"</font> <font color=black> </font>";
//}

//QString Server::text_color_red(QString str)
//{
//    return "<font size=5 color=red>"+str+"</font> <font color=black> </font>";
//}

//QString Server::text_color_black(QString string, int fontsize,QString fontstyle)
//{
//    QString str="<font color=black style=\"%1\">%2</font> <font color=black> </font>";
//    return str.arg(fontstyle).arg(string);
//}

QTcpSocket *Server::getCurrentSocket()
{
    QListWidgetItem *item=ui->listWidget_onlineList->currentItem();
    QString addr=item->text().split(":")[0];//获取选中的ip
    QString port=item->text().split(":")[1];//获取选中的端口
    for(int i(0);i<m_ClientList.count();i++){
        QTcpSocket *current=m_ClientList.at(i);
        //在列表里面找到了这个地址和端口
        if(current->peerAddress().toString().contains(addr)&&
                QString::number(current->peerPort()).contains(port))
            return current;
    }
    return nullptr;
}

QStringList Server::getConnectionIpList()
{
    QStringList list;
    for(int i(0);i<ui->listWidget_onlineList->count();i++){
        list.append(ui->listWidget_onlineList->item(i)->text());
    }
    return list;
}

QSettings *Server::getSetting()
{
    QString dir=QDir::currentPath();
    QSettings *settings=new QSettings(QString("%1/config.conf").arg(dir),QSettings::IniFormat);
    return settings;
}

QJsonObject Server::sendPrivateChatData()
{
    QJsonObject json;
    QTcpSocket *current=getCurrentSocket();
    if(current!=nullptr){
        json.insert("userName","Server");
        json.insert("des_addr",getCurrentSocket()->peerAddress().toString());
        json.insert("des_port",QString::number(getCurrentSocket()->peerPort()));
        json.insert("source_addr",QJsonValue(ui->combo_ipList->currentText()));
        json.insert("source_port",QJsonValue(ui->edit_localPort->text()));
    }
    return json;
}

void Server::newConnectionSlot()
{
    //返回挂起的一个Socket对象
    m_Client=m_Server->nextPendingConnection();
    //将新连接的客户端加入列表
    m_ClientList.append(m_Client);
    m_ClientSet.insert(m_Client);
    //创建线程对象
    m_server_thread=new server_thread(m_ClientList,m_ClientSet,m_Client);
    connect(m_server_thread,&server_thread::sendMsgtoServer,this,&Server::recvMessage);
    connect(m_server_thread,&server_thread::updateOnlineListSignal,this,&Server::updateOnlineList);
    connect(this,&Server::transferSignal,m_server_thread,&server_thread::transferSlot);
    m_server_thread->start();

    //ui->text_onlineList->append(m_Client->peerAddress().toString()+":"+QString::number(m_Client->peerPort()));
    ui->text_chat->append(text_color_red(QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ddd"))));
    ui->text_chat->append(m_Client->peerAddress().toString()+":"+QString::number(m_Client->peerPort())+"已连接");
    //遍历已连接的客户端
    paddingOnlineList();
    //    connect(m_Client,&QTcpSocket::readyRead,this,&Server::readyReadSlot);//接收客户端信息
    //    connect(m_Client,&QTcpSocket::disconnected,this,&Server::disconnectSlot);//断开与客户端的连接
}

void Server::readyReadSlot()
{
    QByteArray readAllData;
    QTcpSocket* current=nullptr;
    QJsonObject jsonObject;

    bool publicFlag=true;
    //如果客户端列表不为空
    if(!m_ClientList.isEmpty()){
        for (int i(0);i<m_ClientList.count() ;i++ ) {
            current=m_ClientList.at(i);
            //如果可以从客户端读入输入返回True
            if(current->isReadable()){
                readAllData = current->readAll();
                QString recvInfo(readAllData);
                qDebug()<<"read->>>>>>"+readAllData;
                jsonObject=convertQStringToJsonObject(recvInfo);

                if(readAllData.isEmpty())
                    continue;
                //本地GBK转Unicode 解决乱码
                QString strTitle = "<"+jsonObject.value("msgType").toString()+">"+"["+jsonObject.value("userName").toString()+"]"+QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ddd"));
                ui->text_chat->append(text_color_blue(strTitle));
                QString strMsg=jsonObject.value("data").toString();
                ui->text_chat->append(strMsg);
                break;
            }
        }
    }


    //转发消息
    //如果是消息类型,必然是群聊，所以不用判断消息范围
    if(jsonObject.value("type").toString().indexOf("msg")>-1){
        for(int index = 0;index < m_ClientList.count();index ++)
        {

            QTcpSocket* temp = m_ClientList.at(index);
            if(current == temp){
                continue;
            }
            if(jsonObject.value("userName").toString().indexOf("Server")>-1)
                continue;
            if(temp->isWritable())
            {
                qDebug()<<"exec transit";
                jsonObject.insert("source_addr",current->peerAddress().toString());
                jsonObject.insert("source_port",QString::number(current->peerPort()));
                temp->write(convertQJsonObjectToQString(jsonObject).toLocal8Bit());
            }


        }

    }
    //如果是文件类型，可能是私聊，可能是群聊
    else if(jsonObject.value("type").toString().indexOf("file")>-1){
        qDebug()<<"type==file";
        if(jsonObject.value("scope").toString().indexOf("private")>-1){
            for(int index = 0;index < m_ClientList.count();index ++)
            {

                QTcpSocket* temp = m_ClientList.at(index);
                QString ip=temp->peerAddress().toString();
                QString port=QString::number(temp->peerPort());
                if(port.indexOf(jsonObject.value("des_port").toString())>-1
                        && ip.indexOf(jsonObject.value("des_addr").toString()>-1)){
                    temp->write(convertQJsonObjectToQString(jsonObject).toLocal8Bit());
                }
            }
        }else if(jsonObject.value("scope").toString().indexOf("public")>-1){
            forward(current,jsonObject);
        }
    }
    if(QString::fromLocal8Bit(readAllData).indexOf("ACK")>-1){
        qDebug()<<"exec ACK";
        //如果范围是私有
        if(jsonObject.value("scope").toString().indexOf("private")>-1){
            for(int index = 0;index < m_ClientList.count();index ++)
            {
                QTcpSocket* temp = m_ClientList.at(index);
                QString ip=temp->peerAddress().toString();
                QString port=QString::number(temp->peerPort());
                if(current == temp){
                    qDebug()<<"current="<<current->peerPort();
                    continue;
                }
                if(jsonObject.value("userName").toString().indexOf("Server")>-1)
                    continue;
                if(port.indexOf(jsonObject.value("des_port").toString())>-1
                        && ip.indexOf(jsonObject.value("des_addr").toString()>-1)){
                    if(temp->isWritable())
                    {
                        qDebug()<<"exec transit file";
                        temp->write(readAllData);
                    }
                }
            }
            //如果范围是公共的
        }else if(jsonObject.value("scope").toString().indexOf("public")>-1){
            forward(current,jsonObject);
        }
    }
}


void Server::disconnectSlot()
{
    //    qDebug()<<m_Client->peerPort();
    //    QMessageBox::information(nullptr,"information","与客户端断开连接");
    ui->text_chat->append(text_color_red(QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ddd"))));
    ui->text_chat->append(m_Client->peerAddress().toString()+":"+QString::number(m_Client->peerPort())+"断开连接");

    m_ClientList.removeOne(m_Client);
    //    qDebug()<<m_ClientList.count();
    paddingOnlineList();
}

void Server::actionDisconnectionSlot()
{
    QTcpSocket *current=getCurrentSocket();
    if(current!=nullptr){
        current->disconnectFromHost();
    }

}


void Server::customContextMenuRequestedSlot(const QPoint &pos)
{
    Q_UNUSED(pos)

    int index=ui->listWidget_onlineList->currentRow();
    if(index>-1){
        QMenu *menu=new QMenu(this);
        QAction *actionDisconnection=new QAction("断开连接");
        //把菜单项加入Menu
        menu->addAction(actionDisconnection);
        //为菜单项连接槽
        //主动断开连接
        connect(actionDisconnection,&QAction::triggered,this,&Server::actionDisconnectionSlot);
        menu->exec(QCursor::pos());
    }
}

void Server::recvCloseWindowSlot()
{
    forward(toSendUntransferData());
}

void Server::recvMessage(QJsonObject jsonObject)
{
    qDebug()<<jsonObject;
    QString strTitle = "<"+jsonObject.value("msgType").toString()+">"+"["+jsonObject.value("userName").toString()+"]"+QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ddd"));
    ui->text_chat->append(text_color_blue(strTitle));
    QString strMsg=jsonObject.value("data").toString();
    ui->text_chat->append(strMsg);
}

void Server::updateOnlineList()
{
    paddingOnlineList();
}



void Server::on_btn_listen_clicked()
{
    //初始化协议
    QHostAddress Address = QHostAddress(ui->combo_ipList->currentText());
    if(m_Server->isListening()){
        m_Server->close();
        m_ClientList.clear();
        ui->btn_listen->setText("监听");
        ui->btn_monitor->setEnabled(false);

    }else{
        //监听地址和端口的传入连接,成功返回True
        if(m_Server->listen(Address,ui->edit_localPort->text().toInt())){
            ui->btn_listen->setText("取消监听");
            ui->btn_monitor->setEnabled(true);

            paddingOnlineList();
        }else{
            QMessageBox::information(nullptr,"information","监听失败！"+m_Server->errorString());
        }

    }
}


void Server::on_btn_send_clicked()
{

    if(!m_ClientList.isEmpty()){
        QString strTitle = "[Server]"+QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ddd"));
        QString strData=ui->edit_send->toPlainText();
        forward(convertQJsonObjectToQString(toSendData()).toLocal8Bit());
        ui->text_chat->append(text_color_blue(strTitle));
        ui->text_chat->append(text_color_black(strData));
        ui->edit_send->clear();
    }

}


//void Server::on_btn_sendFile_clicked()
//{
//    QFileDialog *fileDialog=new QFileDialog(this);
//    fileDialog->setFilter(QDir::Filters::enum_type::Files);
//    QString str_fileName=fileDialog->getOpenFileName();
//    if(!str_fileName.isEmpty()){
//        m_file=new QFile(str_fileName);
//        QByteArray arr = getFileContent(m_file);
//        m_file->close();
//        QString dataHeader = convertQJsonObjectToQString(toSendFileHeader(QFileInfo(str_fileName).fileName(),arr.size()));
//        forward(dataHeader.toLocal8Bit());
//    }
//}






void Server::on_btn_monitor_clicked()
{
    if(ui->listWidget_onlineList->count()>0){
        Welcome *welcom=new Welcome(getConnectionIpList());
        //    welcom->setWindowModality(Qt::ApplicationModal);
        forward(toSendCmd());
        //如果视频监控窗体关闭，通知本窗口
        connect(welcom,&Welcome::closeWindowSignal,this,&Server::recvCloseWindowSlot);
        welcom->show();
    }
}


void Server::on_btn_userManager_clicked()
{
    UserManager *userManager=new UserManager(this);
    //设置为模态窗口，独占UI
    userManager->setWindowModality(Qt::WindowModality::ApplicationModal);
    userManager->show();

}


//void Server::on_btn_sendPic_clicked()
//{
//    QFileDialog *fileDialog=new QFileDialog;
//    QString openFileName = fileDialog->getOpenFileName(nullptr,"","","*.jpg *.bmp *.png");
//    qDebug()<<openFileName;
//}

