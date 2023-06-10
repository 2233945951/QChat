#ifndef WELCOME_H
#define WELCOME_H

#include <QDialog>
#include <QToolButton>
#include <QUdpSocket>
#include "mulitimain.h"
#include <QMainWindow>
#include <QGridLayout>
#include <QSettings>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QScrollArea>
#include <QSet>
#include "tools.h"
#define SEAT_MAXCOUNT 40
#define ONLINE 0
#define OFFLINE 1
#define MONITOR 2
#define EXCEPTION_OFFLINE 3
namespace Ui {
class Welcome;
}

class Welcome : public QMainWindow
{
    Q_OBJECT

public:
    explicit Welcome(QWidget *parent = nullptr);
    explicit Welcome(QStringList list,QWidget *parent = nullptr);
    ~Welcome();




public:
    void searchOnline();

    QSettings *getSettings();

    void modifiChangeFunction();//参数修改
    void monitorFunction();//监听
    void Init_MenuBar();//初始化菜单
    void Init_UI(int column=8);//初始化界面
    void Init_UI(QStringList list);//初始化界面
    void repeatToolButton(int column=8);//遍历QToolButton
    void repeatToolButton(QStringList list);//遍历QToolButton
    QToolButton* getToolButton();//获取一个QToolButton
    QToolButton* getToolButton(QString);//获取一个QToolButton
    void removeGridLayoutItem();//删除布局元素
    void show_MulitWindow(QString title,quint32 ip);//显示监听窗口
    void setMulitClick(QObject*obj);//判断两次点击是不是同一个按钮，并且打开监控窗口
    void InitSets();//初始化存储地址的Set
signals:
    void closeWindowSignal();
private slots:
    void SlotReadyRead();//接受信息
    void on_MulitClick();//QToolButton单击事件
public slots:
    void recvChildCloseEvent(int id);//监听监听窗口是否关闭
    void recvExceptionOffline(int id);//监听客户端是否异常掉线
    void recvRecinnect(int id);
    void customContextMenuRequestedSlot(const QPoint &pos);//右键菜单
    void triggeredSlot();
    void show_SeatSettingWindow();//显示席位修改界面

private:
    Ui::Welcome *ui;
    QUdpSocket *m_socket;
    QMenu *menu;
    QAction *parameterChange;
    QAction *monitor;
    QGridLayout *g_layout=NULL;
    QWidget *childWindow=NULL;
    MulitiMain *muliti=NULL;
    QObject *m_obj=NULL;
    bool isWindowOpen;//判断监听窗口是否已经显示
    int ToolButtonCount=0;//按钮个数
    QTimer *timer=NULL;//计时器
    QSet<QString> addrSets;//存储ip

protected:
    void closeEvent(QCloseEvent *event);
};

#endif // WELCOME_H
