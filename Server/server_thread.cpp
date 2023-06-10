#include "server_thread.h"
#include <QTcpSocket>
#include <QDebug>
#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QHostAddress>
#include <Python.h>
server_thread::server_thread(QList<QTcpSocket*> &list,QSet<QTcpSocket*> &set,QTcpSocket* tcp,QObject *parent)
    : QThread(parent),m_tcp(tcp),m_list(list),m_set(set)
{


}

void server_thread::forward(QTcpSocket *, QJsonObject jsonObject)
{
    for(int index = 0;index < m_list.count();index ++)
    {

        QTcpSocket* temp = m_list.at(index);
        if(m_tcp == temp){
            continue;
        }
        if(jsonObject.value("userName").toString().indexOf("Server")>-1)
            continue;
        if(temp->isWritable())
        {
            jsonObject.insert("source_addr",m_tcp->peerAddress().toString());
            jsonObject.insert("source_port",QString::number(m_tcp->peerPort()));
            temp->write(convertQJsonObjectToQString(jsonObject).toLocal8Bit());
        }
    }
}

void server_thread::forward(QByteArray arr)
{
    if(!m_list.isEmpty()){
        for (int i(0);i<m_list.count() ;i++ ) {
            if(m_list[i]->isWritable()){
                m_list[i]->write(arr);
            }
        }
    }
}

void server_thread::forward(QTcpSocket *, QByteArray arr)
{
    for(int index = 0;index < m_list.count();index ++)
    {

        QTcpSocket* temp = m_list.at(index);
        if(m_tcp == temp){
            continue;
        }
        if(temp->isWritable())
        {
            temp->write(arr);
        }
    }
}

bool server_thread::authentication(QTcpSocket * socket)
{
    for(auto i:m_list){
        if(socket==i)
            return true;
    }
    return false;
}

QTcpSocket *server_thread::find_client(QString addr, QString port)
{
    for(const auto &i:m_list){
        if(i->peerAddress().toString().indexOf(addr)>-1)
            if(QString::number(i->peerPort()).indexOf(port)>-1)
                return i;
    }
    return NULL;
}

bool server_thread::send_code(QString email)
{
    qDebug()<<"send_code";
    QFile *file=new QFile("temp");
    file->open(QFile::WriteOnly);
    qint64 localWrite = file->write(email.toStdString().data());
    qDebug()<<localWrite;
    file->close();
    Py_Initialize();
    if(!Py_IsInitialized()){
        qDebug()<<"init error";
        return false;
    }
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./py')");

    PyObject *module = PyImport_ImportModule("sendAuthCode");
    if(!module){
        qDebug()<<"import error";
        return false;
    }
    PyObject *fun = PyObject_GetAttrString(module,"call");
    //PyObject_CallObject(module,NULL);
    PyObject_CallFunction(fun,NULL);
    return true;
}



void server_thread::run()
{
    //检测是否可以接受数据
    connect(m_tcp,&QTcpSocket::readyRead,this,&server_thread::readyReadSlot);
    connect(m_tcp,&QTcpSocket::disconnected,this,&server_thread::disconnectedSlot);
}
/** 服务端接收函数
 * @brief server_thread::readyReadSlot
 */
void server_thread::readyReadSlot()
{
    if(authentication(m_tcp)){
        QByteArray readAllData=m_tcp->readAll();
        if(readAllData.length()<50)
            qDebug()<<readAllData;
        QJsonObject jsonObject;
        QString recvInfo(readAllData);
        jsonObject=convertQStringToJsonObject(recvInfo);
        if(!jsonObject.isEmpty()){
            //转发
            //转发消息
            //如果是消息类型,必然是群聊，所以不用判断消息范围
            if(jsonObject.value("type").toString().indexOf("msg")>-1){
                for(int index = 0;index < m_list.count();index ++)
                {

                    QTcpSocket* temp = m_list.at(index);
                    if(m_tcp == temp){
                        continue;
                    }
                    if(jsonObject.value("userName").toString().indexOf("Server")>-1)
                        continue;
                    if(temp->isWritable())
                    {
                        jsonObject.insert("source_addr",m_tcp->peerAddress().toString());
                        jsonObject.insert("source_port",QString::number(m_tcp->peerPort()));

                        temp->write(convertQJsonObjectToQString(jsonObject).toLocal8Bit());
                    }
                }
                emit sendMsgtoServer(jsonObject);
            }
            //如果是文件类型，可能是私聊，可能是群聊
            else if(jsonObject.value("type").toString().indexOf("file")>-1){
                qDebug()<<"type==file";
                if(jsonObject.value("scope").toString().indexOf("private")>-1){
//                    for(int index = 0;index < m_list.count();index ++)
//                    {
//                        QTcpSocket* temp = m_list.at(index);
//                        QString ip=temp->peerAddress().toString();
//                        QString port=QString::number(temp->peerPort());
//                        if(port.indexOf(jsonObject.value("des_port").toString())>-1
//                                && ip.indexOf(jsonObject.value("des_addr").toString()>-1)){
//                            temp->write(convertQJsonObjectToQString(jsonObject).toLocal8Bit());
//                            private_socket=temp;
//                        }
//                    }
                    private_socket=find_client(m_tcp->localAddress().toString(),QString::number(m_tcp->localPort()));
                }else if(jsonObject.value("scope").toString().indexOf("public")>-1){
                    forward(readAllData);
                }
                scope=jsonObject.value("scope").toString();
            }
            else if(jsonObject.value("type").toString().indexOf("reply")>-1){
                //else if(QString::fromLocal8Bit(readAllData).indexOf("ACK")>-1){
                if(jsonObject.value("replyContent").toString().indexOf("ACK")>-1){
                    qDebug()<<"exec ACK";
                    scope=jsonObject.value("scope").toString();
                    //如果范围是私有
                    if(jsonObject.value("scope").toString().indexOf("private")>-1){
                        for(int index = 0;index < m_list.count();index ++)
                        {

                            QTcpSocket* temp = m_list.at(index);
                            QString ip=temp->peerAddress().toString();
                            QString port=QString::number(temp->peerPort());
                            if(m_tcp == temp){
                                qDebug()<<"current="<<m_tcp->peerPort();
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
                    }
                    //如果范围是公共的
                    else if(jsonObject.value("scope").toString().indexOf("public")>-1){
                        forward(m_tcp,jsonObject);

                    }
                }


            }
            //如果是指令类型
            else if(jsonObject.value("type").toString().indexOf("cmd")>-1){
                //找回密码
                if(jsonObject.value("cmd_type").toString().indexOf("forgot_password")>-1){
                    QString addr=jsonObject.value("source_addr").toString();
                    QString port=jsonObject.value("source_port").toString();
                    QTcpSocket *socket = find_client(addr,port);
                    if(jsonObject.value("data").toString().indexOf("AUTH_CODE")>-1){
                        //发送验证码
                        qDebug()<<"发送验证码";
                        bool send_flag = send_code(jsonObject.value("email").toString());
                        if(send_flag){
                            if(socket){
                                socket->write(toSendAuthCodeReply(true));
                            }
                        }
                        else{
                            if(socket){
                                socket->write(toSendAuthCodeReply(false));
                            }
                        }
                    }
                    else if(jsonObject.value("data").toString().indexOf("RESET_PASSWORD")>-1){
                        QFile file("temp");
                        file.open(QFile::ReadOnly);
                        QString recv_code=jsonObject.value("code").toString();
                        QString auth_code = QString(file.readAll());
                        file.close();
                        if(recv_code.indexOf(auth_code)>-1){

                            if(socket->isWritable()){
                                socket->write(toSendAuthResetReply(true));
                            }
                        }
                        else{
                            if(socket->isWritable()){
                                socket->write(toSendAuthResetReply(false));
                            }
                        }
                    }
                }
                //心跳检测
                else if(jsonObject.value("cmd_type").toString().indexOf("heartbeat")>-1){
                    if(jsonObject.value("data").toString().indexOf("ACK")>-1){
                        qDebug()<<"in ACK";
                        m_set.remove(m_tcp);
                    }
                }
            }
        }
        if(!scope.isEmpty()){
            qDebug()<<scope;
            if(scope.indexOf("private")>-1){
                qDebug()<<"private coccc";
                forward(private_socket,readAllData);
            }else{
                forward(readAllData);
                qDebug()<<"public coccc";
            }
        }

    }
    else
        m_tcp->disconnected();


}
void server_thread::disconnectedSlot()
{
    m_list.removeOne(m_tcp);
    m_set.remove(m_tcp);
    m_tcp->close();
    m_tcp->deleteLater();
    emit updateOnlineListSignal();
}

void server_thread::transferSlot()
{
    forward(m_tcp,toSendCmd());
}














