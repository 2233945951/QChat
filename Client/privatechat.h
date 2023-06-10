#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QDialog>
#include <QJsonObject>
#include <QWidget>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QJsonDocument>
#include <QTcpSocket>
#include <QFileDialog>
#include <QProgressBar>
#include "client_thread.h"
#include "tools.h"
namespace Ui {
class PrivateChat;
}

class PrivateChat : public QDialog
{
    Q_OBJECT

public:
    explicit PrivateChat(QWidget *parent = nullptr);
    explicit PrivateChat(QTcpSocket* socket,QWidget *parent = nullptr);
    ~PrivateChat();

    void Init();
    void Init_fontComboBox();
    void bytesWrittenSlot();
    QJsonObject convertQStringToJsonObject(QString str);
    QString convertQJsonObjectToQString(QJsonObject json);
    QJsonObject toSendData();
    QJsonObject toSendFileHeader(QString filename,int fileSize);
    QJsonObject toSendFileContent(QByteArray arr);
    QJsonObject toSendReply();
    QByteArray getFileContent(QFile*);

private:
    Ui::PrivateChat *ui;
    QTcpSocket  *m_tcp_socket=NULL;
    QUdpSocket *m_udp_socket=NULL;
    bool connectState=false;
    bool isRecv_processbar_display=false;
    QString m_fileName;
    QFile *m_file;
    int m_sendFileSize;
    int m_fileSize;
    QProgressBar *recv_file_processbar;
    Client_Thread *thread;
public slots:
    void recvData(QJsonObject data);
    void readyReadSlot();
    void readyReadTcpSlot();
    void sendFileContentSlot();
    void recvFileContentSlot(int size);
    void recvFileHeader(QJsonObject json);
    void finish_file_transfer();
signals:
    void closeWindow();

private slots:
    void on_btn_send_clicked();
    void on_btn_sendFile_clicked();

protected:
    void closeEvent(QCloseEvent *) override;
};

#endif // PRIVATECHAT_H
