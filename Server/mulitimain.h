#ifndef MULITIMAIN_H
#define MULITIMAIN_H

#include <QDialog>
#include <QUdpSocket>
#include <QTcpServer>
#include <QGridLayout>
#include <QCloseEvent>
const int UDP_MAX_SIZE=4096;

//包头
struct PackageHeader
{
    //包头大小(sizeof(PackageHeader))
    unsigned int uTransPackageHdrSize;
    //当前包的大小(sizeof(PackageHeader)+数据长度)
    unsigned int uTransPackageSize;
    //数据的总大小
    unsigned int uDataSize;
    //数据被分成包的个数
    unsigned int uDataPackageNum;
    //数据包当前的帧号
    unsigned int uDataPackageCurrIndex;
    //数据包在整个数据中的偏移
    unsigned int uDataPackageOffset;
};
//组包结构体
struct UdpUnpackData
{
    char data[1024*1000];
    int length;
};

QT_BEGIN_NAMESPACE
namespace Ui { class MulitiMain; }
QT_END_NAMESPACE

class MulitiMain : public QDialog
{
    Q_OBJECT

public:
    explicit MulitiMain(QString title,quint32 ip,int id,QWidget *parent = nullptr);
    ~MulitiMain();
    QImage get_image_fromByte(const QString &data);
    QByteArray get_image_fromFile(const QImage &image);

protected:
    void closeEvent(QCloseEvent *) override;
    void resizeEvent(QResizeEvent *) override;
private:
    Ui::MulitiMain *ui;
    QUdpSocket *m_udpSocket;
    int recvImageNum;
    UdpUnpackData imageData;
    quint32 m_addr;
    int m_id;
    QString m_title;
    QTimer *m_timer;
    int timeoutNum;
signals:
    void closeWindow(int);
    void exceptionOffline(int);
    void reconnect(int);
public slots:
    void readyReadSlot();
    void reTransmission();
};
#endif // MULITIMAIN_H
