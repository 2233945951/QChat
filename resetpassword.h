#ifndef RESETPASSWORD_H
#define RESETPASSWORD_H

#include <QDialog>

#include <QSqlDatabase>
namespace Ui {
class ResetPassword;
}

class ResetPassword : public QDialog
{
    Q_OBJECT

public:
    explicit ResetPassword(QString username,QWidget *parent = nullptr);
    ~ResetPassword();
    bool checkInput();
    bool connectionDataBase();
private slots:
    void on_btn_commit_clicked();

private:
    Ui::ResetPassword *ui;
    QString m_username;
    QSqlDatabase db;
};

#endif // RESETPASSWORD_H
