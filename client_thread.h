#ifndef CLIENT_THREAD_H
#define CLIENT_THREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QFile>
#include "tools.h"
#include "mulitclient.h"
#include "QProgressBar"
class Client_Thread:public QThread
{
    Q_OBJECT
public:
    explicit Client_Thread(QFile *file,QTcpSocket *tcp=nullptr, QObject *parent = nullptr);
    explicit Client_Thread(QTcpSocket *tcp=nullptr, QObject *parent = nullptr);
    ~Client_Thread();
    QJsonObject toSendReply();
protected:
    void run();
private:
    QTcpSocket *m_tcp;
    QFile *m_file;
    MulitClient *m_mulitClient;
    QString m_fileName;
    QString m_fileScope;
    int m_fileSize;
    int m_sendFileSize;
    QProgressBar *recv_file_progressbar;
signals:
    void sendMsg(QJsonObject);
    void closeSocketSignal();
    void sendFileContentSignal();
    void recvFileContentSignal(int);
    void finish_file_transfer();
    void untransferSignal();
    void sendCodeSignal(bool);
    void sendAuthUserSignal(bool);
    void sendFileHeader(QJsonObject);
public slots:
    void readyReadSlot();
    void disconnectedSlot();
    void bytesWrittenSlot();
};

#endif // CLIENT_THREAD_H
