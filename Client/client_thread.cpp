#include "client_thread.h"
#include "clientmsg.h"

#include <QDir>
#include <QMessageBox>

Client_Thread::Client_Thread(QFile *file, QTcpSocket *tcp, QObject *parent)
    :QThread(parent),m_tcp(tcp),m_file(file)
{
    m_fileSize=0;
    m_sendFileSize=0;
    m_fileScope="";
}

Client_Thread::Client_Thread(QTcpSocket *tcp, QObject *parent)
    :QThread(parent),m_tcp(tcp)
{

}

Client_Thread::~Client_Thread()
{

}

QJsonObject Client_Thread::toSendReply()
{
    QJsonObject *json=new QJsonObject;
    json->insert("scope","public");
    json->insert("type","reply");
    json->insert("replyContent","ACK");
    return *json;
}

void Client_Thread::run()
{
    //检测是否可以接受数据
    connect(m_tcp,&QTcpSocket::readyRead,this,&Client_Thread::readyReadSlot);
    connect(m_tcp,&QTcpSocket::disconnected,this,&Client_Thread::disconnectedSlot);
}

void Client_Thread::readyReadSlot()
{


    QByteArray ReadAll = m_tcp->readAll();
    QString recvInfo(ReadAll);
    QJsonObject recvJson = convertQStringToJsonObject(recvInfo);
    qDebug()<<objectName()<<"readyReadSlot"<<recvJson;
    //如果接收到的是消息类型
    if(!recvJson.isEmpty()){
        if(recvJson.value("type").toString().indexOf("msg")>-1){
            emit sendMsg(recvJson);
        }
        else if(recvJson.value("type").toString().indexOf("file")>-1){
            if(recvJson.value("fileType").toString().indexOf("fileHeader")>-1){
                m_fileName=recvJson.value("fileName").toString();
                m_fileSize=recvJson.value("fileSize").toInt();
                m_fileScope=recvJson.value("scope").toString();
                QString sender_addr=recvJson.value("sender_addr").toString();
                QString sender_port=recvJson.value("sender_port").toString();

                QMessageBox::StandardButton localInformation = QMessageBox::information(nullptr,"信息",sender_addr+":"+sender_port+"发送了文件"+m_fileName+",是否接收？",QMessageBox::StandardButton::Ok,QMessageBox::StandardButton::No);
                qDebug()<<m_fileScope<<m_fileName;
                if(localInformation==QMessageBox::StandardButton::Ok){
                    qDebug()<<"exec qms";
                    QDir *dir=new QDir("./recvFiles");
                    if(!dir->exists())
                        dir->mkdir(dir->path());

                    if(!m_fileName.isEmpty()){
                        qDebug()<<dir->currentPath();
                        m_file=new QFile(dir->path()+"/"+m_fileName);
                        m_file->open(QIODevice::WriteOnly);
                        m_file->close();
                        m_sendFileSize=0;
                        QByteArray arr=convertQJsonObjectToQString(toSendReply()).toLocal8Bit();
                        m_tcp->write(arr);
                        qDebug()<<"send ACK";
                        emit sendFileHeader(recvJson);
                    }
                }
            }
        }
        else if(recvJson.value("replyContent").toString().indexOf("ACK")>-1){
            qDebug()<<"recv ACK";
            emit sendFileContentSignal();
        }
        else if(recvJson.value("type").toString().indexOf("cmd")>-1){
            qDebug()<<"in cmd";
            if(recvJson.value("data").toString().indexOf("transfer")>-1){
                m_mulitClient=new MulitClient;
                connect(this,&Client_Thread::untransferSignal,m_mulitClient,&MulitClient::untransferSlot);

            }
            else if(recvJson.value("data").toString().indexOf("untransfer")>-1){
                qDebug()<<"in untransfer";
                emit untransferSignal();
                delete m_mulitClient;
            }
            else if(recvJson.value("cmd_type").toString().indexOf("forgot_password")>-1){
                qDebug()<<"in forgot_password";
                if(recvJson.value("data").toString().indexOf("CODE_ACK_SUCC")>-1){
                    qDebug()<<"exec sendCodeSignal";
                    emit sendCodeSignal(true);
                }
                else if(recvJson.value("data").toString().indexOf("CODE_ACK_ERR")>-1){
                    emit sendCodeSignal(false);
                }
                else if(recvJson.value("data").toString().indexOf("AUTH_ACK_SUCC")>-1){
                    emit sendAuthUserSignal(true);
                }
                else if(recvJson.value("data").toString().indexOf("AUTH_ACK_Err")>-1){
                    emit sendAuthUserSignal(false);
                }
            }
            else if(recvJson.value("cmd_type").toString().indexOf("heartbeat")>-1){
                qDebug()<<"in heartbeat";
                if(recvJson.value("data").toString().indexOf("SYN")>-1){
                    if(m_tcp->isWritable()){
                        recvJson.insert("data","ACK");
                        QByteArray arr=convertQJsonObjectToQString(recvJson).toLocal8Bit();
                        qDebug()<<m_tcp->write(arr);
                    }
                }
            }
        }
    }
    else if(!m_fileScope.isEmpty()){
        //qDebug()<<"fileContent";
        if(m_sendFileSize<m_fileSize){
            if(!m_file->isOpen())
                m_file->open(QIODevice::WriteOnly);
            m_sendFileSize+= m_file->write(ReadAll);
            qDebug()<<m_sendFileSize<<m_fileSize;
            emit recvFileContentSignal(m_sendFileSize);
            if(m_sendFileSize>=m_fileSize){
                m_file->close();
                m_sendFileSize=0;
                emit finish_file_transfer();
                qDebug()<<"finish";
            }
        }

    }
    QThread::msleep(1);
}

void Client_Thread::disconnectedSlot()
{
    //m_tcp->close();
    emit closeSocketSignal();
    quit();
}

void Client_Thread::bytesWrittenSlot()
{
    if(!m_file->isOpen())
        m_file->open(QIODevice::ReadOnly);
    QByteArray arr=m_file->read(10240);
    if(arr.isEmpty()){
        m_file->close();
        disconnect(m_tcp,&QTcpSocket::bytesWritten,this,&Client_Thread::bytesWrittenSlot);
        return;
    }
    m_tcp->write(arr);
}


