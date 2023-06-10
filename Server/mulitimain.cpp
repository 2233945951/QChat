#include "mulitimain.h"
#include "ui_mulitimain.h"
#include <QBuffer>
#include <QHostAddress>
#include <QDesktopWidget>
#include <QThread>
#include <QTimer>
#include <welcome.h>
#include <QCloseEvent>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>
MulitiMain::MulitiMain(QString title, quint32 ip,int id, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MulitiMain)
{
    ui->setupUi(this);

    m_udpSocket=new QUdpSocket(this);
    m_id=id;
    m_addr=ip;
    m_title=title;
    m_udpSocket->bind(54321,QUdpSocket::ShareAddress);
 //   m_udpSocket->bind(8888,QUdpSocket::BindMode::enum_type::DefaultForPlatform);
    setWindowFlags(Qt::Window);

    m_udpSocket->setReadBufferSize(0);

    setWindowTitle("");
    connect(m_udpSocket,&QUdpSocket::readyRead,this,&MulitiMain::readyReadSlot);
    m_timer=new QTimer;
    //发送一个探测包
    reTransmission();
    //超时未回复重发探测包
    connect(m_timer,&QTimer::timeout,this,&MulitiMain::reTransmission);
    m_timer->start(1000);




    //    for(int i=0;i<3;i++){
    //        QTimer time;
    //        QString s="transfer";
    //        m_udpSocket->writeDatagram(s.toLocal8Bit().data(),s.toLocal8Bit().size(),QHostAddress(addr),8888);
    //        time.start(5000);
    //    }

}

MulitiMain::~MulitiMain()
{
    delete m_udpSocket;
    delete ui;
}

QImage MulitiMain::get_image_fromByte(const QString &data)
{
    QByteArray imageData=QByteArray::fromBase64(data.toLatin1());
    QImage image;
    image.loadFromData(imageData);
    return image;
}

QByteArray MulitiMain::get_image_fromFile(const QImage &image)
{
    QByteArray imageData;
    QBuffer buffer(&imageData);
    image.save(&buffer,"jpg");
    imageData=imageData.toBase64();
    return imageData;
}



void MulitiMain::closeEvent(QCloseEvent *event)
{
    //    qDebug()<<"close";
    //    connect(this,&MulitiMain::closeWindow,this,[](){});
    emit closeWindow(m_id);
    m_timer->stop();
    disconnect(m_udpSocket,&QUdpSocket::readyRead,this,&MulitiMain::readyReadSlot);
    delete m_udpSocket;
    event->accept();
}

void MulitiMain::resizeEvent(QResizeEvent *)
{

    ui->label->resize(size());
}


void MulitiMain::readyReadSlot()
{

    //    QtConcurrent::run([=](){
    //收到回复，计时器停止
    m_timer->stop();
    timeoutNum=0;

    QByteArray datagram;
    while (m_udpSocket->hasPendingDatagrams())
    {
        datagram.resize(m_udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        m_udpSocket->readDatagram(
                    datagram.data(),
                    datagram.size(),
                    &sender,
                    &senderPort);
        if(datagram.contains("offline")){
            //            QMessageBox msg(QMessageBox::Warning,"警告",)
        }else{
            //组包开始+++++++++++++++++++++++++++
            static int num = 0;
            static uint size = 0;
            PackageHeader *packageHead = (PackageHeader *)datagram.data();
            if (packageHead->uDataPackageCurrIndex == num)
            {
                num++;
                //数据部分=包的总大小-包的表头部分
                size += packageHead->uTransPackageSize-packageHead->uTransPackageHdrSize;
                if (size > 1024*1000)
                {
                    qDebug()<<"image too big";
                    num = 1;
                    size = 0;
                    //                    return;
                    continue;
                }
                if (packageHead->uDataPackageOffset > 1024*1000)
                {
                    qDebug()<<"image too big";
                    packageHead->uDataPackageOffset = 0;
                    num = 1;
                    size = 0;
                    //                    return;
                    continue;
                }
                //qDebug()<<size << packageHead->uDataSize;
                memcpy(imageData.data+packageHead->uDataPackageOffset, datagram.data()+packageHead->uTransPackageHdrSize,
                       packageHead->uTransPackageSize-packageHead->uTransPackageHdrSize);

                if ((packageHead->uDataPackageNum == packageHead->uDataPackageCurrIndex)
                        && (size == packageHead->uDataSize))
                {
                    imageData.length = packageHead->uDataSize;

                    QImage image;
                    image.loadFromData((uchar *)imageData.data,imageData.length,"JPG");

                    QPixmap pixmap=QPixmap::fromImage(image);


                    ui->label->setPixmap(pixmap.scaled(ui->label->size(),Qt::AspectRatioMode::IgnoreAspectRatio,Qt::TransformationMode::SmoothTransformation));
                    setWindowTitle(QString("%1(%2)-已连接").arg(m_title).arg(QHostAddress(m_addr).toString()));
                    recvImageNum++;
                    emit reconnect(m_id);
                    memset(&imageData,0,sizeof(UdpUnpackData));
                    num = 1;
                    size = 0;
                    if(datagram.size()){
                        m_udpSocket->writeDatagram("ACK",3,QHostAddress(m_addr),8888);
                        //                        qDebug()<<"write ack";
                    }

                }
            }
            else
            {
                num = 1;
                size = 0;
                memset(&imageData,0,sizeof(UdpUnpackData));
            }
            //组包结束+++++++++++++++++++++++++++
            //        QThread::msleep(150);
        }

        //    });

    }


    m_timer->start(100);

}

void MulitiMain::reTransmission()
{
    if(timeoutNum>=3){
        ui->label->setPixmap(QPixmap(":/image/black.jpg"));
        setWindowTitle(QString("%1(%2)-未连接").arg(m_title,QHostAddress(m_addr).toString()));
        emit exceptionOffline(m_id);
    }
    QString s="transfer";
    m_udpSocket->writeDatagram(s.toLocal8Bit().data(),s.toLocal8Bit().size(),QHostAddress(m_addr),8888);
    timeoutNum++;

}



