#include "usermanager.h"
#include "ui_usermanager.h"

#include <QMessageBox>
#include <QSettings>

UserManager::UserManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserManager)
{
    ui->setupUi(this);
    Init();
}

UserManager::~UserManager()
{
    delete ui;
    delete tableView;
    delete model;
}

void UserManager::Init()
{
    if(connectionDataBase()){
        tableView=new QTableView(this);
//        tableView->setItemDelegate(4,new QEditTime);
        tableView->setGeometry(0,0,width(),height());

        model=new QSqlRelationalTableModel(this,db);
        model->setTable("ClientUsers");
        tableView->setModel(model);
        model->select();
        qDebug()<<model->rowCount();
        tableView->show();



    }else{
        QMessageBox::warning(nullptr,"警告","数据库连接失败！"+db.lastError().text());
    }
}

bool UserManager::connectionDataBase()
{
    db=QSqlDatabase::addDatabase("QMYSQL");
    QSettings *seting=getSetting();
    db.setHostName(seting->value("/connect/IP").toString());
    db.setDatabaseName(seting->value("/connect/DATABASE_QT").toString());
    db.setPort(3306);
    db.setUserName(seting->value("/connect/USERNAME").toString());
    db.setPassword(seting->value("/connect/PASSWORD").toString());
    return db.open();
}
