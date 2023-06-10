#ifndef MULITCLIENT_H
#define MULITCLIENT_H

#include <QCloseEvent>
#include <QObject>
#include <QTimer>
#include <QUdpSocket>
#include "tools.h"

struct PackageHeader

{
    unsigned int uTransPackageHdrSize; //包头大小

    unsigned int uTransPackageSize; //当前包大小

    unsigned int uDataSize; //数据总大小

    unsigned int uDataPackageNum; //数据被分成包的个数

    unsigned int uDataPackageCurrIndex; //数据包当前的帧号

    unsigned int uDataPackageOffset; //数据包在整个数据中的偏移
};

const int UDP_MAX_SIZE=4096;  //定义一个包发送的数据大小
class MulitClient : public QObject
{
    Q_OBJECT
public:
    explicit MulitClient(QObject *parent = nullptr);
    void sendUdpData();

private:
    QUdpSocket *m_socket;
    int sendImageNum;
    QTimer *timer;
public slots:
    void readyReadSlot();
    void SlotTriggered();
    void reTansmission();
    void untransferSlot();
protected:
    void closeEvent(QCloseEvent *event);

};

#endif // MULITCLIENT_H
