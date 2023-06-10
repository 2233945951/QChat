#include "client_thread.h"
#include "privatechat.h"
#include "ui_privatechat.h"

#include <QBuffer>
#include <QJsonValue>
#include <QDateTime>
PrivateChat::PrivateChat(QWidget *parent)
    :QDialog(parent),
      ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
    //    this->setWindowFlags(Qt::Window|Qt::WindowStaysOnTopHint);

    Init();

}

PrivateChat::PrivateChat(QTcpSocket *socket, QWidget *parent)
    :QDialog(parent),
      ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
    Init();
    m_tcp_socket=socket;

    thread=new Client_Thread(m_tcp_socket);
    connect(thread,&Client_Thread::sendFileContentSignal,this,&PrivateChat::sendFileContentSlot);
    connect(thread,&Client_Thread::recvFileContentSignal,this,&PrivateChat::recvFileContentSlot);
    connect(thread,&Client_Thread::sendFileHeader,this,&PrivateChat::recvFileHeader);
    connect(thread,&Client_Thread::finish_file_transfer,this,&PrivateChat::finish_file_transfer);
    //    connect(m_tcp_socket,&QTcpSocket::readyRead,this,&PrivateChat::readyReadTcpSlot);
    thread->start();
}

PrivateChat::~PrivateChat()
{
    qDebug()<<"执行析构";
    delete thread;
    delete ui;
}

void PrivateChat::Init()
{
    Init_fontComboBox();
    ui->label_sourceAddr->setVisible(false);
    ui->label_sourcePort->setVisible(false);
    m_udp_socket=new QUdpSocket(this);
}

void PrivateChat::Init_fontComboBox()
{

    for(int i(1);i<=72;i++){
        ui->comboBox_fontSize->addItem(QString::number(i));
    }

}

void PrivateChat::bytesWrittenSlot()
{
    if(!m_file->isOpen())
        m_file->open(QIODevice::ReadOnly);
    QByteArray arr=m_file->read(40960);
    if(arr.isEmpty()){
        m_file->close();
        disconnect(m_tcp_socket,&QTcpSocket::bytesWritten,this,&PrivateChat::bytesWrittenSlot);
        return;
    }
    if(m_tcp_socket->isWritable())
        m_sendFileSize+=m_tcp_socket->write(arr);
    qDebug()<<m_sendFileSize<<QFileInfo(*m_file).size();

}

QJsonObject PrivateChat::convertQStringToJsonObject(QString str)
{
    QJsonDocument jsonDocument=QJsonDocument::fromJson(str.toLocal8Bit());
    if(!jsonDocument.isNull()){
        return jsonDocument.object();
    }
    return *new QJsonObject;
}

QString PrivateChat::convertQJsonObjectToQString(QJsonObject json)
{
    return QJsonDocument(json).toJson();
}

QJsonObject PrivateChat::toSendData()
{
    //要发送：主机名，IP地址，传送的数据，用户名
    QJsonObject *jsonObject=new QJsonObject;
    jsonObject->insert("userName",ui->label_username->text());
    jsonObject->insert("desAddr",QJsonValue(ui->label_addr->text()));
    jsonObject->insert("desPort",QJsonValue(ui->label_port->text()));
    jsonObject->insert("data",QJsonValue(ui->edit_send->text()));
    return *jsonObject;
}

QJsonObject PrivateChat::toSendFileHeader(QString filename, int fileSize)
{
    //要经过tcp服务端进行转发，所以必须指定目标客户的ip地址和端口号
    QJsonObject *json=new QJsonObject;
    json->insert("scope","private");
    json->insert("type","file");
    json->insert("fileType","fileHeader");
    json->insert("fileName",filename);
    json->insert("fileSize",fileSize);
    json->insert("des_addr",ui->label_addr->text());
    json->insert("des_port",ui->label_port->text());
    return *json;
}

QJsonObject PrivateChat::toSendFileContent(QByteArray arr)
{
    //要经过tcp服务端进行转发，所以必须指定目标客户的ip地址和端口号
    QJsonObject *json=new QJsonObject;
    json->insert("scope","private");
    json->insert("type","file");
    json->insert("fileType","fileContent");
    json->insert("fileData",QString::fromLocal8Bit(arr));
    json->insert("des_addr",ui->label_addr->text());
    json->insert("des_port",ui->label_port->text());
    return *json;
}

QJsonObject PrivateChat::toSendReply()
{
    QJsonObject *json=new QJsonObject;
    json->insert("scope","private");
    json->insert("type","reply");
    json->insert("replyContent","ACK");
    json->insert("des_addr",ui->label_addr->text());
    json->insert("des_port",ui->label_port->text());
    return *json;
}

QByteArray PrivateChat::getFileContent(QFile *file)
{
    file->open(QFile::OpenModeFlag::ReadWrite);
    QByteArray localReadAll = file->readAll();
    file->close();
    return localReadAll;
}

void PrivateChat::recvData(QJsonObject data)
{
    ui->label_username->setText(data.value("userName").toString());
    ui->label_addr->setText(data.value("des_addr").toString());
    ui->label_port->setText(data.value("des_port").toString());

    ui->label_sourceAddr->setText(data.value("source_addr").toString());
    ui->label_sourcePort->setText(data.value("source_port").toString());

    connect(m_udp_socket,&QUdpSocket::readyRead,this,&PrivateChat::readyReadSlot);
    m_udp_socket->bind(ui->label_sourcePort->text().toInt()%10000,QUdpSocket::ShareAddress);

}

void PrivateChat::readyReadSlot()
{
    qDebug()<<"readyReadSlot";
    QByteArray recvByteArray;
    QHostAddress targetAddr;
    quint16 targetPort;
    while (m_udp_socket->hasPendingDatagrams()) {
        recvByteArray.resize(m_udp_socket->pendingDatagramSize());
        m_udp_socket->readDatagram(recvByteArray.data(),recvByteArray.size(),&targetAddr,&targetPort);
    }
    QString recvStr(recvByteArray);
    QJsonObject jsonObject = convertQStringToJsonObject(recvStr);
    QString text_title = "["+jsonObject.value("userName").toString()+"]"+QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
    QString text_content=jsonObject.value("data").toString();   //本地GBK转Unicode 解决乱码

    text_title=text_color_red(text_title);

    ui->text_chat->append(text_title);
    ui->text_chat->append(text_content);
}

void PrivateChat::readyReadTcpSlot()
{
    qDebug()<<"readyReadTcpSlot";
    QByteArray recv_byteArr=m_tcp_socket->readAll();
    QString recv_Str = QString::fromLocal8Bit(recv_byteArr);
    QJsonObject json=convertQStringToJsonObject(recv_Str);
    qDebug()<<recv_Str;
    if(json.value("type").toString().indexOf("file")>-1){
        if(json.value("fileType").toString().indexOf("fileHeader")>-1){
            qDebug()<<QFileInfo(m_fileName).suffix();
            QFileDialog *fileDialog=new QFileDialog(this,"","",QFileInfo(m_fileName).suffix());
            fileDialog->setWindowTitle("保存");
            m_fileName=json.value("fileName").toString();
            fileDialog->selectFile(m_fileName);
            QString saveFileName = fileDialog->getSaveFileName(this,tr(""),m_fileName);
            m_fileName=saveFileName;
            if(!saveFileName.isEmpty()){

                m_tcp_socket->write(convertQJsonObjectToQString(toSendReply()).toLocal8Bit());
                qDebug()<<"send ACK";
            }

        }
        else if(json.value("fileType").toString().indexOf("fileContent")>-1){
            QByteArray arr = json.value("fileData").toString().toLocal8Bit();
            qDebug()<<m_fileName;
            QFile *file=new QFile(m_fileName);
            file->open(QFile::OpenModeFlag::ReadWrite);
            file->write(arr);
            file->close();
        }
    }
    else if(json.value("replyContent").toString().indexOf("ACK")>-1){
        qDebug()<<"recv ACK";
        QString dataContent=convertQJsonObjectToQString(toSendFileContent(getFileContent(m_file)));
        m_tcp_socket->write(dataContent.toLocal8Bit().data(),dataContent.toLocal8Bit().size());
    }


}

void PrivateChat::sendFileContentSlot()
{
    connect(m_tcp_socket,&QTcpSocket::bytesWritten,this,&PrivateChat::bytesWrittenSlot);
    bytesWrittenSlot();
}

void PrivateChat::recvFileContentSlot(int size)
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

void PrivateChat::recvFileHeader(QJsonObject json)
{
    m_fileName=json.value("fileName").toString();
    m_fileSize=json.value("fileSize").toInt();
}

void PrivateChat::finish_file_transfer()
{
    recv_file_processbar->close();
    delete recv_file_processbar;
    isRecv_processbar_display=false;
}

void PrivateChat::on_btn_send_clicked()
{

    m_udp_socket->writeDatagram(convertQJsonObjectToQString(toSendData()).toLocal8Bit(),
                                QHostAddress(ui->label_addr->text()),ui->label_port->text().toInt()%10000);
    QString text_title = "["+ui->label_username->text()+"]"+QString(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));//本地GBK转Unicode 解决乱码
    QString text_content=ui->edit_send->text();

    text_title=text_color_blue(text_title);

    ui->text_chat->append(text_title);
    ui->text_chat->append(text_content);
    ui->edit_send->clear();
}


void PrivateChat::on_btn_sendFile_clicked()
{
    QFileDialog *fileDialog=new QFileDialog(this);
    fileDialog->setFilter(QDir::Filters::enum_type::Files);
    QString str_fileName=fileDialog->getOpenFileName();
    if(!str_fileName.isEmpty()){
        m_file=new QFile(str_fileName);
        QString dataHeader = convertQJsonObjectToQString(toSendFileHeader(QFileInfo(str_fileName).fileName(),QFileInfo(str_fileName).size()));
        if(m_tcp_socket->isOpen() && m_tcp_socket->isValid()){
            m_tcp_socket->write(dataHeader.toLocal8Bit());
        }
        m_sendFileSize=0;
    }
}

void PrivateChat::closeEvent(QCloseEvent *)
{
    qDebug()<<"触发closeEvent";
    m_tcp_socket->disconnect();
    emit closeWindow();
}

