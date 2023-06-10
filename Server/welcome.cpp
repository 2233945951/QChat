#include "welcome.h"
#include "ui_welcome.h"
#include "mulitimain.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTimer>
#include <QToolButton>
#include <QMessageBox>
#include <QMenu>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QDir>
#include <QRegExpValidator>
#include <QTextCodec>
#include <QJsonObject>
#include <QJsonDocument>
Welcome::Welcome(QStringList list, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Welcome)
{
    ui->setupUi(this);
    setWindowTitle("视频监控");
    Init_MenuBar();
    //Init_UI();
    Init_UI(list);

    m_socket=new QUdpSocket(this);
    m_socket->bind(8888,QUdpSocket::ShareAddress);
    connect(m_socket,&QUdpSocket::readyRead,this,&Welcome::SlotReadyRead);

    //    timer=new QTimer(this);
    //    connect(timer,&QTimer::timeout,this,&Welcome::searchOnline);
    //    searchOnline();



    //    InitSets();
    //    timer->start(5000);

    //    QScrollArea *scrool=new QScrollArea(this);
    //    scrool->setWidget(this);

}

Welcome::~Welcome()
{
    delete m_socket;
    delete menu;
    delete parameterChange;
    delete g_layout;
    delete ui;
}

void Welcome::Init_UI(int column)
{
    if(childWindow)
        delete childWindow;
    //    if(g_layout)
    //        delete g_layout;


    childWindow=new QWidget(this);
    g_layout=new QGridLayout(childWindow);
    g_layout->setMargin(0);
    repeatToolButton(column);
    childWindow->setLayout(g_layout);

    setCentralWidget(childWindow);


}

void Welcome::Init_UI(QStringList list)
{
    if(childWindow)
        delete childWindow;
    childWindow=new QWidget(this);
    g_layout=new QGridLayout(childWindow);
    g_layout->setMargin(0);
    repeatToolButton(list);
    childWindow->setLayout(g_layout);

    setCentralWidget(childWindow);
}

void Welcome::repeatToolButton(int column)
{
    ToolButtonCount=0;
    //    removeGridLayoutItem();

    for(int i=1;i<=SEAT_MAXCOUNT/column+1;i++){
        if(ToolButtonCount<SEAT_MAXCOUNT){
            for(int j=1;j<=column;j++){
                if(ToolButtonCount<SEAT_MAXCOUNT){
                    //获取一个QToolButton
                    QToolButton *toolBtn=getToolButton();
                    connect(toolBtn,&QToolButton::clicked,this,&Welcome::on_MulitClick);
                    connect(toolBtn,&QWidget::customContextMenuRequested,this,&Welcome::customContextMenuRequestedSlot);
                    g_layout->addWidget(toolBtn,i,j);
                    ToolButtonCount++;
                }else
                    break;
            }
        }
        else
            break;
    }
    qDebug()<<g_layout->rowCount()<<g_layout->columnCount();
    for(int j(1);j<g_layout->columnCount();j++){
        QLabel *l1=new QLabel("第"+QString::number(j)+"列");
        l1->setMaximumSize(100,20);
        l1->setMinimumSize(100,20);
        l1->setFont(QFont("黑体",10));
        l1->setAlignment(Qt::AlignmentFlag::AlignCenter);
        g_layout->addWidget(l1,0,j);
    }
    for(int i(1);i<g_layout->rowCount();i++){
        QLabel *l1=new QLabel("第"+QString::number(i)+"排");
        l1->setMaximumSize(50,100);
        l1->setMinimumSize(50,100);
        l1->setFont(QFont("黑体",10));

        l1->setAlignment(Qt::AlignmentFlag::AlignCenter);
        g_layout->addWidget(l1,i,0);
    }
}
void Welcome::repeatToolButton(QStringList list)
{
    for(int i(0);i<8;i++){
        if(g_layout->count()<list.count()){
            for(int j=0;j<list.count();j++){
                if(g_layout->count()<list.count()){
                    //获取一个QToolButton
                    QToolButton *toolBtn=getToolButton(list.at(j));
                    connect(toolBtn,&QToolButton::clicked,this,&Welcome::on_MulitClick);
                    g_layout->addWidget(toolBtn,i,j);
                }
                else
                    break;
            }
        }
        else
            break;
    }
}

void Welcome::Init_MenuBar()
{
    QMenuBar *menuBar=new QMenuBar(ui->frame);
    menuBar->resize(width(),height());
    QMenu *menu=new QMenu;
    QAction *seatSettingAction=new QAction;
    QAction *exitAction=new QAction;

    menu->setTitle("设置");
    seatSettingAction->setText("席位设置");
    exitAction->setText("退出");



    //点击退出菜单，退出程序
    connect(exitAction,&QAction::triggered,this,&QWidget::close);
    //弹出席位修改窗口
    connect(seatSettingAction,&QAction::triggered,this,&Welcome::show_SeatSettingWindow);

    menuBar->addMenu(menu);
    menu->addAction(seatSettingAction);
    menu->addAction(exitAction);
    setMenuBar(menuBar);




}


QToolButton *Welcome::getToolButton()
{
    QString dir = QDir::currentPath();
    QSettings *setting=getSettings();
    QToolButton *toolBtn=new QToolButton(this);
    //    toolBtn->resize(120,60);
    toolBtn->setMaximumSize(100,80);
    toolBtn->setMinimumSize(100,80);
    toolBtn->setIcon(QIcon(":/images/mulit/offline.png"));
    toolBtn->setIconSize(QSize(32,32));
    //    toolBtn->setStyleSheet("background:rgba(0,0,0,0)");
    toolBtn->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    toolBtn->setProperty("id",ToolButtonCount+1);
    toolBtn->setProperty("index",g_layout->count()+1);
    toolBtn->setProperty("status",OFFLINE);
    QString hostName=setting->value(QString("Client%1/HostName").arg(ToolButtonCount+1)).toString();
    QString hostIp=setting->value(QString("Client%1/HostIp").arg(ToolButtonCount+1)).toString();


    //单行字符大于10，换行
    for(int i(0);i<hostName.length();i++)
        if(i%10==0 && i>0)
            hostName.insert(i,"\n");

    toolBtn->setText(hostIp+"\n"+hostName);
    toolBtn->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

    return toolBtn;
    delete setting;
}

QToolButton *Welcome::getToolButton(QString str)
{
    QToolButton *toolBtn=new QToolButton(this);
    //    toolBtn->resize(120,60);
    toolBtn->setMaximumSize(100,80);
    toolBtn->setMinimumSize(100,80);
    //    toolBtn->setIcon(QIcon(":/images/mulit/offline.png"));
    toolBtn->setIcon(QIcon(":/images/mulit/online.png"));
    toolBtn->setIconSize(QSize(32,32));
    //    toolBtn->setStyleSheet("background:rgba(0,0,0,0)");
    toolBtn->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    toolBtn->setProperty("id",g_layout->count());
    toolBtn->setProperty("status",ONLINE);
    toolBtn->setText(str.split(":")[0]+"\n"+str.split(":")[1]);
    return toolBtn;
}

void Welcome::removeGridLayoutItem()
{
    for(int i(0);i<g_layout->count();i++)
        delete g_layout->itemAt(i);
}

void Welcome::show_MulitWindow(QString title, quint32 ip)
{
    //    int id=sender()->property("id").toInt();
    int index=sender()->property("index").toInt();

    if(muliti== NULL){
        muliti=new MulitiMain(title,ip,index,this);
        muliti->show();
        isWindowOpen=true;
    }else{
        const QRect localGeometry = muliti->geometry();
        if(isWindowOpen){
            muliti->close();
        }
        muliti=new MulitiMain(title,ip,index,this);
        muliti->setGeometry(localGeometry);
        muliti->show();
        isWindowOpen=true;
    }

    connect(muliti,&MulitiMain::closeWindow,this,&Welcome::recvChildCloseEvent);
    connect(muliti,&MulitiMain::exceptionOffline,this,&Welcome::recvExceptionOffline);
    connect(muliti,&MulitiMain::reconnect,this,&Welcome::recvRecinnect);
    if(muliti->exec()==QDialog::Accepted){}
}

void Welcome::setMulitClick(QObject *obj)
{
    QString className=obj->metaObject()->className();
    if(className.indexOf("QToolButton")>-1){
        QToolButton *toolBtn=dynamic_cast<QToolButton*>(obj);
        quint32 ip=QHostAddress(toolBtn->text().split("\n")[0]).toIPv4Address();
        qDebug()<<ip<<toolBtn->text();
        toolBtn->setIcon(QIcon(":/images/mulit/monitering.png"));

        show_MulitWindow(toolBtn->text(),ip);
    }
}

void Welcome::InitSets()
{
    addrSets.clear();
    for(int i(0);i<g_layout->count();i++){
        QString className=g_layout->itemAt(i)->widget()->metaObject()->className();
        if(className.indexOf("QToolButton")>-1){
            QToolButton* btn=dynamic_cast<QToolButton*>(g_layout->itemAt(i)->widget());
            addrSets.insert(btn->text().split("\n")[0]);
            //            QHostAddress addr(btn->text().split("\n")[0]);
        }else
            continue;
    }
}

void Welcome::show_SeatSettingWindow()
{
    Qt::WindowFlags flags;
    flags|=Qt::WindowType::WindowCloseButtonHint;

    QDialog *w=new QDialog(this);
    QFormLayout *f_layout=new QFormLayout;

    w->setWindowTitle("席位修改");





    QLineEdit *columnSetting=new QLineEdit;
    QRegExpValidator* regExpValid=new QRegExpValidator;
    QRegExp regExp("[1-9]{2}");
    regExpValid->setRegExp(regExp);

    columnSetting->setValidator(new QIntValidator(1,15));
    columnSetting->setText(QString::number(g_layout->columnCount()-1));

    QPushButton *commit=new QPushButton;
    commit->setText("确定");

    connect(commit,&QPushButton::clicked,this,[=](){
        Init_UI(columnSetting->text().toInt());
        w->close();
    });

    f_layout->addRow("每行的席位个数：",columnSetting);
    f_layout->addRow(commit);
    w->setLayout(f_layout);

    w->show();

}

void Welcome::closeEvent(QCloseEvent *event)
{


    QString s="untransfer";

    m_socket->writeDatagram(s.toLocal8Bit(),QHostAddress::Broadcast,8888);

    QWidget::closeEvent(event);

    emit closeWindowSignal();

}



void Welcome::searchOnline()
{
    QString s="online";
    if(!isWindowOpen){
        for(int i(0);i<g_layout->count();i++){
            QString className=g_layout->itemAt(i)->widget()->metaObject()->className();
            if(className.indexOf("QToolButton")>-1){
                QToolButton* btn=dynamic_cast<QToolButton*>(g_layout->itemAt(i)->widget());
                //                btn->setIcon(QIcon(":/images/mulit/offline.png"));
                QHostAddress addr(btn->text().split("\n")[0]);
                m_socket->writeDatagram(s.toLocal8Bit(),s.toLocal8Bit().size(),addr,8888);
            }else
                continue;
        }
        //客户端非正常断线
        for(int i=0;i<g_layout->count();i++){
            QString className=g_layout->itemAt(i)->widget()->metaObject()->className();
            if(className.indexOf("QToolButton")>-1){
                QToolButton *btn=dynamic_cast<QToolButton*>(g_layout->itemAt(i)->widget()) ;
                if(addrSets.contains(btn->text().split("\n")[0])){
                    btn->setIcon(QIcon(":/images/mulit/offline.png"));
                    btn->setProperty("status",OFFLINE);
                }
            }
        }
        InitSets();
    }
    //        time->start(3000);
    //    }



}

QSettings* Welcome::getSettings()
{
    QString dir = QDir::currentPath();
    QSettings *setting=new QSettings(QString("clientConfig.para"),QSettings::IniFormat);
    setting->setIniCodec(QTextCodec::codecForName("UTF-8"));
    return setting;
}

void Welcome::modifiChangeFunction()
{
    //    timer->stop();
    QSettings *setting=getSettings();
    QDialog *modifiWindow=new QDialog;
    QFormLayout *f_layout=new QFormLayout;
    QVBoxLayout *v_layout=new QVBoxLayout;
    QLineEdit *HostName;
    QLineEdit *HostIp;

    HostName=new QLineEdit;
    HostIp=new QLineEdit;

    QString str_HostName=setting->value(QString("Client%1/HostName").arg(sender()->property("id").toString())).toString();
    HostName->setText(str_HostName);


    QString str_HostIp=setting->value(QString("Client%1/HostIp").arg(sender()->property("id").toString())).toString();
    HostIp->setText(str_HostIp);
    QRegExp regExp("^((1?[1-9]?\\d|[1-2][0-4]\\d|25[0-5])\\.){3}(1?[1-9]?\\d|[1-2][0-4]\\d|25[0-5])$");
    QRegExpValidator *regExpValid=new QRegExpValidator(regExp);
    HostIp->setValidator(regExpValid);

    f_layout->addRow(new QLabel("席位名称"),HostName);
    f_layout->addRow(new QLabel("席位地址"),HostIp);

    QPushButton *commit=new QPushButton("提交");
    v_layout->addLayout(f_layout);
    v_layout->addWidget(commit);
    commit->setProperty("id",sender()->property("id").toInt());
    commit->setProperty("index",sender()->property("index").toInt());
    connect(commit,&QPushButton::clicked,this,[=](){
        QSettings *setting=getSettings();
        int id=sender()->property("id").toInt();
        int index=sender()->property("index").toInt();
        qDebug()<<id<<index<<sender()->metaObject()->className();
        if(HostName->text().isEmpty())
            QMessageBox::information(nullptr,"信息","席位名称不能为空！");
        else if(HostIp->text().isEmpty())
            QMessageBox::information(nullptr,"信息","席位地址不能为空！");
        for(int i=0;i<g_layout->count();i++){
            QString className=g_layout->itemAt(i)->widget()->metaObject()->className();
            if(className.indexOf("QToolButton")>-1){
                QToolButton *toolBtn=dynamic_cast<QToolButton*>(g_layout->itemAt(i)->widget());
                //自己不能和自己去比较
                if(id!=toolBtn->property("id").toInt()){
                    //若btn的text文本里的ip地址和toolBtn的ip相同
                    if(HostIp->text().contains(toolBtn->text().split("\n")[0])){
                        QMessageBox msg;
                        msg.setWindowTitle("警告");
                        msg.setIcon(QMessageBox::Icon::Warning);
                        msg.setText("您输入的IP地址不能和其他席位相同！");
                        QPushButton *ok=msg.addButton("是(Y)",QMessageBox::ButtonRole::YesRole);
                        ok->setShortcut(Qt::Key_Y);
                        msg.exec();
                        return ;
                    }
                }
            }
        }
        QString className=g_layout->itemAt(sender()->property("index").toInt()-1)->widget()->metaObject()->className();
        if(setting->isWritable()){
            if(className.indexOf("QToolButton")>-1){
                setting->setValue(QString("Client%1/HostName").arg(id),HostName->text());
                setting->setValue(QString("Client%1/HostIp").arg(id),HostIp->text());
                QToolButton *toolBtn=dynamic_cast<QToolButton*>(g_layout->itemAt(sender()->property("index").toInt()-1)->widget());
                toolBtn->setText(HostIp->text()+"\n"+HostName->text());
            }
        }
        delete setting;
        modifiWindow->close();
        //        timer->start(1000);
    });


    modifiWindow->resize(300,100);
    modifiWindow->setWindowTitle("席位参数修改");
    modifiWindow->setLayout(v_layout);
    modifiWindow->show();
    delete setting;
}

void Welcome::monitorFunction()
{
    int index=sender()->property("id").toInt();
    QToolButton* btn= dynamic_cast<QToolButton*>(g_layout->itemAt(index-1)->widget());
    quint32 ip=QHostAddress(btn->text().split("\n")[0]).toIPv4Address();
    QString title=btn->text().split("\n")[1];
    btn->setIcon(QIcon(":/images/mulit/monitering.png"));
    btn->setProperty("status",MONITOR);
    show_MulitWindow(title,ip);
}





void Welcome::SlotReadyRead()
{
    QByteArray arr;
    QHostAddress targetAddr;
    quint16 targetPort;
    while (m_socket->hasPendingDatagrams()) {
        arr.resize(m_socket->pendingDatagramSize());
        m_socket->readDatagram(arr.data(),arr.size(),&targetAddr,&targetPort);
        QString recv(arr);
        QString strAddr=QHostAddress(targetAddr.toIPv4Address()).toString();
        if(recv.contains("online")){
            for(int i=0;i<g_layout->count();i++){
                QString className=g_layout->itemAt(i)->widget()->metaObject()->className();
                if(className.indexOf("QToolButton")>-1){
                    QToolButton *btn=dynamic_cast<QToolButton*>(g_layout->itemAt(i)->widget()) ;
                    if(btn->text().contains(strAddr)){
                        if(btn->property("status").toInt()==OFFLINE){
                            btn->setIcon(QIcon(":/images/mulit/online.png"));
                            btn->setProperty("status",ONLINE);
                            addrSets.remove(btn->text().split("\n")[0]);
                        }

                    }
                }
            }
        }
        //客户端正常断线
        else if(recv.contains("offline")){

            for(int i=0;i<g_layout->count();i++){
                QString className=g_layout->itemAt(i)->widget()->metaObject()->className();
                if(className.indexOf("QToolButton")>-1){
                    QToolButton *btn=dynamic_cast<QToolButton*>(g_layout->itemAt(i)->widget()) ;
                    if(btn->text().contains(strAddr)){
                        btn->setIcon(QIcon(":/images/mulit/offline.png"));
                        btn->setProperty("status",OFFLINE);
                        addrSets.remove(strAddr);
                    }
                }
            }
        }
    }

}






void Welcome::on_MulitClick()
{

    if((m_obj==NULL||m_obj!=sender()||!isWindowOpen)&&sender()->property("status").toInt()==ONLINE){
        m_obj=sender();
        setMulitClick(m_obj);
    }
}

void Welcome::recvChildCloseEvent(int id)
{
    //    ui->TestSeat_2->setIcon(QIcon(":/images/mulit/online.png"));
    qDebug()<<id;
    QToolButton *toolBtn=dynamic_cast<QToolButton*>(g_layout->itemAt(id)->widget());
    toolBtn->setIcon(QIcon(":/images/mulit/online.png"));
    isWindowOpen=false;
}
void Welcome::recvExceptionOffline(int id)
{
    QToolButton *toolBtn=dynamic_cast<QToolButton*>(g_layout->itemAt(id)->widget());
    toolBtn->setIcon(QIcon(":/images/mulit/exception_offline.png"));

}

void Welcome::recvRecinnect(int id)
{
    QToolButton *toolBtn=dynamic_cast<QToolButton*>(g_layout->itemAt(id)->widget());
    toolBtn->setIcon(QIcon(":/images/mulit/monitering.png"));
}

void Welcome::customContextMenuRequestedSlot(const QPoint &pos)
{
    Q_UNUSED(pos)
    menu=new QMenu(this);
    parameterChange=new QAction("参数修改",menu);
    parameterChange->setObjectName("parameterChange");
    monitor=new QAction("监视屏幕",menu);
    monitor->setObjectName("monitor");
    parameterChange->setProperty("id",sender()->property("id"));
    parameterChange->setProperty("index",sender()->property("index"));
    monitor->setProperty("id",sender()->property("id"));
    connect(parameterChange,&QAction::triggered,this,&Welcome::triggeredSlot);
    connect(monitor,&QAction::triggered,this,&Welcome::triggeredSlot);

    QToolButton *toolBtn=dynamic_cast<QToolButton*>(sender());
    if(toolBtn->property("status").toInt()==ONLINE){
        menu->clear();
        menu->addAction(parameterChange);
        menu->addAction(monitor);
    }
    if(toolBtn->property("status").toInt()==OFFLINE){
        menu->clear();
        menu->addAction(parameterChange);
    }
    menu->exec(QCursor::pos());
}

void Welcome::triggeredSlot()
{
    if(sender()->objectName().contains("parameterChange")){
        modifiChangeFunction();
    }
    else if(sender()->objectName().contains("monitor"))
    {
        monitorFunction();
    }


    //    if(modifiWindow->exec()==QDialog::Accepted){}


}


