#ifndef CLIENTMSG_H
#define CLIENTMSG_H

#include <QDialog>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QHostInfo>
#include <QJsonObject>
#include <QTableWidgetItem>
#include <QJsonObject>
#include "customqtable.h"
#include "privatechat.h"
#include "client_thread.h"
#include <QSettings>
#include <QSqlDatabase>
#include <QFormLayout>
#include <QVBoxLayout>
QT_BEGIN_NAMESPACE
namespace Ui { class ClientMsg; }
QT_END_NAMESPACE

class ClientMsg : public QDialog
{
    Q_OBJECT

public:
    ClientMsg(QWidget *parent = nullptr);
    ~ClientMsg();

public:
    QJsonObject toSendMsgData();//待发送的数据
    void paddingToOnlineTable(QJsonObject json);
    void Init_fontComboBox();
    void Init_Signal_Slots();
    QJsonObject toSendFileHeader(QString filename,int fileSize);
//    QJsonObject toSendFileContent(QByteArray arr);
    QJsonObject toSendReply();
    QByteArray getFileContent(QFile*);
    bool connectionDataBase();
    QString text_color_black(QString string);
    bool text_isBold();
    bool text_isItalic();
    bool text_isUnderline();

private:
    QJsonObject sendPrivateChatData(QStringList data);
private slots:
    void disconnSlot();
    void on_btn_send_clicked();
    void on_btn_connect_clicked();
    void on_tableWidget_onlineList_itemDoubleClicked(QTableWidgetItem *item);
    void on_btn_sendFile_clicked();
    void on_btn_italic_clicked();
    void on_btn_underLine_clicked();
    void on_btn_bold_clicked();
    void on_btn_clearRecord_clicked();
    void on_btn_saveRecord_clicked();
    void on_btn_modifyInfo_clicked();

public slots:
    void reciveInfo(QString str);
    void recvClosePrivateWindow();
    void bytesWrittenSlot();
    void recvMsg(QJsonObject json);
    void closeSocketSlot();
    void sendFileContentSlot();
    void recvFileContentSlot(int size);
    void recvFileHeader(QJsonObject json);
    void finish_file_transfer();
signals:
    void sendData(QJsonObject);
    void sendSocket(QTcpSocket*);
private:
    Ui::ClientMsg *ui;
    QTcpSocket *socket;
    bool socketState;
    bool isItalic=false;
    bool isBold=false;
    bool isUnderline=false;
    bool isRecv_processbar_display=false;

    QFile *m_file;
    QString m_fileName;
    int m_fileSize;
    int m_sendFileSize;
    QList<QJsonObject> jsonList;
    QDialog *modifyWindow;
    QSqlDatabase db;
    QProgressBar *recv_file_processbar;
    Client_Thread *thread;

};
#endif // CLIENTMSG_H
