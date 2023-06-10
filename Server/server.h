#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include <QNetworkInterface>
#include <QTcpServer>
#include <QTcpSocket>
#include <QListWidgetItem>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QHostInfo>
#include <QSqlDatabase>
#include <QSettings>
#include <QFile>
#include "server_thread.h"
#include "tools.h"
QT_BEGIN_NAMESPACE
namespace Ui { class Server; }
QT_END_NAMESPACE

class Server : public QWidget
{
    Q_OBJECT

public:
    Server(QWidget *parent = nullptr);
    ~Server();
    void Init();
    void initComboIpEnum();
    void initComboBoxFontsize();
    void initConection();
    void paddingOnlineList();

//    QJsonObject convertQStringToJsonObject(QString str);
//    QString convertQJsonObjectToQString(QJsonObject json);
    QJsonObject toSendData();//待发送的数据
    QJsonObject toSendFileHeader(QString filename,int fileSize);
    QByteArray getFileContent(QFile *file);
    QByteArray toSendCmd();
    QByteArray toSendUntransferData();
    bool connectionDatabase();
    void forward(QTcpSocket*,QJsonObject);
    void forward(QByteArray);
    void heartbreat_confirmation();

    QTcpSocket* getCurrentSocket();
    QStringList getConnectionIpList();
    QSettings *getSetting();
private:
    Ui::Server *ui;
    QTcpServer *m_Server;
    QTcpSocket *m_Client;
    QList<QTcpSocket*> m_ClientList;
    QSet<QTcpSocket*> m_ClientSet;
    QListWidgetItem *m_ClientItem;
    QSqlDatabase db;
    QString addr;
    QString port;
    QFile *m_file;
    server_thread *m_server_thread;
private:
    QJsonObject sendPrivateChatData();
signals:
    void sendData(QJsonObject);
    void transferSignal();

public slots:
    void newConnectionSlot();
    void readyReadSlot();
    void disconnectSlot();//客户端断开连接触发
    void actionDisconnectionSlot();//右键断开连接
    void customContextMenuRequestedSlot(const QPoint &pos);
    void recvCloseWindowSlot();
    void recvMessage(QJsonObject);
    void updateOnlineList();
    void heartbeat_confirmation_slot();
private slots:
    void on_btn_listen_clicked();
    void on_btn_send_clicked();
    void on_btn_monitor_clicked();
    void on_btn_userManager_clicked();
};
#endif // SERVER_H
