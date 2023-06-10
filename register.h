#ifndef REGISTER_H
#define REGISTER_H

#include <QDialog>
#include <QSqlDatabase>
namespace Ui {
class Register;
}

class Register : public QDialog
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = nullptr);
    ~Register();

    void Init_Ctrl();
    bool connectionDataBase();
    bool checkInput();

private slots:
    void on_btn_submit_clicked();
private:
    Ui::Register *ui;
    QSqlDatabase db;

};

#endif // REGISTER_H
