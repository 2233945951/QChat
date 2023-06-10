#ifndef SERVER_THREAD_H
#define SERVER_THREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QList>
#include <QSet>
#include "tools.h"
#include <QTimer>

class server_thread : public QThread
{
    Q_OBJECT
public:
    explicit server_thread(QList<QTcpSocket*> &list,QSet<QTcpSocket*> &set,QTcpSocket* tcp=nullptr,QObject *parent = nullptr);
public:

    void forward(QTcpSocket*,QJsonObject);
    void forward(QByteArray);
    void forward(QTcpSocket*,QByteArray);
    bool authentication(QTcpSocket*);
    QTcpSocket* find_client(QString addr,QString port);
    bool send_code(QString email);

protected:
    void run() override;
public slots:
    void readyReadSlot();
    void disconnectedSlot();
    void transferSlot();
private:
    QTcpSocket* m_tcp;
    QList<QTcpSocket*> &m_list;
    QSet<QTcpSocket*> &m_set;
    QString scope;
    QTcpSocket *private_socket;


signals:
    void sendMsgtoServer(QJsonObject);
    void updateOnlineListSignal();
};

#endif // SERVER_THREAD_H
