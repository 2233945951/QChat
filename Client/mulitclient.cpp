#include "mulitclient.h"

#include <QApplication>
#include <QPixmap>
#include <QDesktopWidget>
#include <QScreen>
#include <QPainter>
#include <QBuffer>
#include <QThread>
MulitClient::MulitClient(QObject *parent) : QObject(parent)
{
    QSettings *setting=getSettings();
    quint16 server_port=setting->value("/SERVER/PORT").toInt();
    m_socket=new QUdpSocket;
    m_socket->bind(server_port,QUdpSocket::ShareAddress);
    connect(m_socket,&QUdpSocket::readyRead,this,&MulitClient::readyReadSlot);
    delete setting;
}

void MulitClient::sendUdpData()
{
    QPixmap tempPixmap = QApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId());
//    QPixmap cursors(":/cursors.cur");
//    QPainter *painter=new QPainter(&tempPixmap);
//    painter->drawPixmap(QCursor::pos().x(),QCursor::pos().y(),20,20,cursors);

    QSettings *setting=getSettings();
    QString server_ip=setting->value("/SERVER/IP").toString();
    //quint16 client_port=setting->value("/CLIENT/PORT").toInt();

    QBuffer buffer;
    tempPixmap.save(&buffer,"jpg");
    int dataLength=buffer.data().size();//数据长度

    unsigned char *dataBuffer=(unsigned char *)buffer.data().data();//获取到图片的数据

    int packetNum = 0;//包个数
    int lastPaketSize = 0;//最后一个包的大小
    packetNum = dataLength / UDP_MAX_SIZE;//数据长度/每个包的大小
    lastPaketSize = dataLength % UDP_MAX_SIZE;
    int currentPacketIndex = 0;//包索引

    if (lastPaketSize != 0)
    {
        packetNum = packetNum + 1;
    }

    PackageHeader packageHead;

    packageHead.uTransPackageHdrSize=sizeof(packageHead);
    packageHead.uDataSize = dataLength;//每个数据包的大小
    packageHead.uDataPackageNum = packetNum;//包的个数
    unsigned char frameBuffer[1024*1000];

    memset(frameBuffer,0,1024*1000);//初始化

    while (currentPacketIndex < packetNum)
    {
        if (currentPacketIndex < (packetNum-1))//如果不是最后一个包
        {
            //包大小=包头大小+数据大小
            packageHead.uTransPackageSize = sizeof(PackageHeader)+UDP_MAX_SIZE;
            packageHead.uDataPackageCurrIndex = currentPacketIndex+1;
            packageHead.uDataPackageOffset = currentPacketIndex*UDP_MAX_SIZE;//偏移
            memcpy(frameBuffer, &packageHead, sizeof(PackageHeader));//把表头数据复制到该内存区域
            //复制数据部分到frameBuffer
            memcpy(frameBuffer+sizeof(PackageHeader), dataBuffer+packageHead.uDataPackageOffset, UDP_MAX_SIZE);

            int len=m_socket->writeDatagram(
                        (const char*)frameBuffer, packageHead.uTransPackageSize,
                        QHostAddress(server_ip), 54321);

            if((unsigned int)len!=packageHead.uTransPackageSize)
            {
                qDebug()<<"Failed to send image";
            }

            currentPacketIndex++;
        }
        else{
            packageHead.uTransPackageSize = sizeof(PackageHeader)+(dataLength-currentPacketIndex*UDP_MAX_SIZE);
            packageHead.uDataPackageCurrIndex = currentPacketIndex+1;
            packageHead.uDataPackageOffset = currentPacketIndex*UDP_MAX_SIZE;
            memcpy(frameBuffer, &packageHead, sizeof(PackageHeader));
            memcpy(frameBuffer+sizeof(PackageHeader), dataBuffer+packageHead.uDataPackageOffset, dataLength-currentPacketIndex*UDP_MAX_SIZE);
            int len=m_socket->writeDatagram(
                        (const char*)frameBuffer, packageHead.uTransPackageSize,
                        QHostAddress(server_ip),54321);

            if((unsigned int)len!=packageHead.uTransPackageSize)
            {
                qDebug()<<"Failed to send image";
            }

            currentPacketIndex++;

        }
        QThread::msleep(100);

    }
    sendImageNum++;
    delete setting;
   // delete painter;
}

void MulitClient::readyReadSlot()
{
    QSettings *setting=getSettings();
    QString server_ip=setting->value("/SERVER/IP").toString();
    delete setting;

    while (m_socket->hasPendingDatagrams()) {
        QByteArray recv;
        recv.resize(m_socket->pendingDatagramSize());
        m_socket->readDatagram(recv.data(),recv.size(),new QHostAddress(server_ip));
        if(recv.size()){
            QString info(recv);
            qDebug()<<info;
            if(info.contains("ACK")){
                sendUdpData();
            }
            else if(info.contains("online")){
                QString s="online";
                m_socket->writeDatagram(s.toLocal8Bit(),s.toLocal8Bit().size(),QHostAddress(server_ip),8888);

                //                delete painter;
            }
            else if(info.contains("transfer")){
                sendUdpData();
            }
            else if(info.contains("untransfer")){
                timer->stop();

            }
        }
    }
    timer->start(1000);
}

void MulitClient::SlotTriggered()
{

}

void MulitClient::reTansmission()
{
    QSettings *setting=getSettings();
    QString server_ip=setting->value("/SERVER/IP").toString();
    quint64 server_port=setting->value("/SERVER/PORT").toInt();
    QString s="online";
    m_socket->writeDatagram(s.toLocal8Bit(),s.toLocal8Bit().size(),QHostAddress(server_ip),server_port);

    delete setting;
}

void MulitClient::untransferSlot()
{
    qDebug()<<"已收到untransfer指令,停止时钟";
    timer->stop();
}

void MulitClient::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    timer->stop();
}
