#ifndef FORGOT_PASSWORD_H
#define FORGOT_PASSWORD_H
#include <QTcpSocket>
#include <QDialog>
#include <QJsonObject>
#include "tools.h"
#include "client_thread.h"
#include <QSqlDatabase>
namespace Ui {
class Forgot_Password;
}

class Forgot_Password : public QDialog
{
    Q_OBJECT

public:
    explicit Forgot_Password(QWidget *parent = nullptr);
    ~Forgot_Password();
    QByteArray toSendData();
    QByteArray toSendAuthCode();
    void Init_Socket();
    bool connectionDataBase();
    bool Authentication();
    bool checkInput();
private slots:
    void on_btn_sendCode_clicked();



    void on_btn_commit_clicked();

public slots:
    void timeout_slot();
    void recvCodeSlot(bool flag);
    void recvCompareSlot(bool flag);
    void closeSocketSlot();
private:
    Ui::Forgot_Password *ui;
    quint16 timeout_count;
    QTimer *timer;
    QTcpSocket *m_tcp;
    bool status;
    QSqlDatabase db;

};

#endif // FORGOT_PASSWORD_H
