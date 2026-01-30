#include "menu.h"
#include "ui_menu.h"
#include "gamewin.h"

unsigned WINAPI connect_thread(void *arg);

Menu::Menu(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Menu)
{
    ui->setupUi(this);

    this->setFixedSize(600, 600);
    setWindowTitle("五子棋对战");
    setWindowIcon(QPixmap(":/new/prefix1/img/Title1.png"));
    //ui->stackedWidget->setStyleSheet("border-image: url(:/new/prefix1/img/menubg1.png);");
    ui->stackedWidget->setCurrentIndex(0);

    tableModel = new QStandardItemModel(this);
    ui->tableView->setModel(tableModel);
    //columnTitle << "房间信息" << "IP地址" <<"套接字"<<"进入房间";
    columnTitle << "房间信息" << "IP地址" << "进入房间";
    tableModel->setHorizontalHeaderLabels(columnTitle);

    client = new client_net();              //初始化客户端网络

    ui->tableView->setColumnWidth(0,265);
    ui->tableView->setColumnWidth(1,130);
    //ui->tableView->setColumnWidth(2,50);
    ui->tableView->setColumnWidth(2,150);

}

Menu::~Menu()
{
    qDebug() << "主页面析构~" <<Qt::endl;
    delete client;          //释放网络信息传输对象内存
    delete tableModel;
    delete ui;

}

void Menu::timerEvent(QTimerEvent *event)
{
    if(client->connect_thread_running)      //正在连接
    {
        ui->connect_stat_label->setStyleSheet("QLabel{color:blue}");
        ui->connect_stat_label->setText("<正在连接服务器...>");
        return;
    }
    else if(client->isConnected())          //已经连接
    {
        ui->create_btn->setDisabled(false);     //激活创建对局按钮
        ui->refresh_btn->setDisabled(false);    //激活刷新对局按钮
        ui->connect_stat_label->setStyleSheet("QLabel{color:green}");
        ui->connect_stat_label->setText("已连接服务器-<创建或加入对局>");
        return;
    }
    else                                    //没连接
    {
        ui->create_btn->setDisabled(true);          //[BUG] setDisable不是setDefault
        ui->refresh_btn->setDisabled(true);
        ui->connect_stat_label->setStyleSheet("QLabel{color:red}");
        ui->connect_stat_label->setText("<请连接服务器>");
        return;
    }
}

//本地游戏按钮点击事件
void Menu::on_local_game_btn_clicked()
{
    GameWin *local_game = new GameWin();
    local_game->show();         //展示本地游戏主界面
    this->hide();               //隐藏菜单界面
    //游戏结束信号发出后显示主菜单并释放空间
    connect(local_game, &GameWin::gameOver, [=](){
        this->show();
        delete local_game;
    });
}


void Menu::on_about_btn_clicked()
{
    QMessageBox::information(this, "作者", "制作人：清兵卫@github \n本项目开源供大家学习参考", QMessageBox::Ok);
}

//退出游戏按钮点击事件
void Menu::on_exit_btn_clicked()
{
    close();
}

//网络游戏按钮
void Menu::on_net_game_btn_clicked()
{
    timerId = startTimer(500);              //启动定时器（500ms调用一次timerEvent）
    on_reconnect_btn_clicked();             //进入页面首先连接服务器
    ui->stackedWidget->setCurrentIndex(1);
    ui->create_btn->setDisabled(true);
    ui->refresh_btn->setDisabled(true);
}

//返回主页面
void Menu::on_exit_net_btn_clicked()
{
    tableModel->removeRows(0, tableModel->rowCount());       //清空房间列表
    killTimer(timerId);
    if(client->connect_thread_running)
        client->connect_thread_running = false;
    if(client->isConnected())
        client->disconnect();
    ui->stackedWidget->setCurrentIndex(0);              //切回主页面
}

//连接服务器按钮功能
void Menu::on_reconnect_btn_clicked()
{
    if(client->isConnected())
        return;
    //开启连接线程
    HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, connect_thread, (void*)client, 0, NULL);
}

//连接线程函数
unsigned WINAPI connect_thread(void *arg)
{
    client_net *clnt = (client_net*)arg;
    qDebug() << "连接线程开始..." << Qt::endl;
    clnt->connect();
    qDebug() << "连接线程结束..." <<Qt::endl;
}

//创建房间按钮功能实现
void Menu::on_create_btn_clicked()
{
    if(!client->isConnected())
        return;
    QString create_str = "C:" + ui->LineEdit->text();       //房间名
    int ret = client->send_msg(create_str);
    if(ret == SOCKET_ERROR)
    {
        QMessageBox::information(this, "网络对战", "创建失败,请重试", QMessageBox::Ok);
        return;
    }

    client->clear();
    //将client对象通过实参传入
    internet_game *inter_game = new internet_game(this->x(),this->y(),client,ui->LineEdit->text());
    inter_game->show();                 //进入网络对战棋盘
    this->hide();

    //绑定游戏结束事件
    connect(inter_game, &internet_game::gameOver, [=](){
        qDebug() << "网络游戏结束" << Qt::endl;
        delete inter_game;
        this->show();           //显示菜单
    });
}

//刷新战局按钮
void Menu::on_refresh_btn_clicked()
{
    client->clear();
    if(!client->isConnected())
        return ;
    client->send_msg("R");                          //给服务器发送刷新请求
    Sleep(300);
    int people=client->get_msg().toInt();           //在消息队列中获取已连接的客户端数
    ui->people_label->setText(QString("在线人数:%1").arg(people));
    string str=client->get_msg().toStdString();        //获取没有人加入的空闲房间
    int sum=0;
    for(int i=0;i<str.size();i++)
        sum=sum*10+str[i]-'0';                              //字符串转数字
    tableModel->removeRows(0,tableModel->rowCount());       //先清空列表(初始化)
    for(int i=0;i<sum;i++)                          //遍历列表显示每个空闲房间信息
    {
        QString get;
        QPushButton *button = new QPushButton("开战");

        get=client->get_msg();
        tableModel->setItem(i, 0, new QStandardItem(get));  //显示房间名
        button->setProperty("Name",get);        //设置属性

        get=client->get_msg();
        tableModel->setItem(i, 1, new QStandardItem(get));  //显示IP地址
        button->setProperty("Ip",get);

        get=client->get_msg();
        //tableModel->setItem(i, 2, new QStandardItem(get));  //显示套接字
        button->setProperty("Fd",get);

        ui->tableView->setIndexWidget(tableModel->index(i, 2), button);
        connect(button,&QPushButton::pressed,this,&Menu::join_game);    //绑定join点击事件
    }
    if(sum==0)
        ui->tableView->setColumnWidth(0,265);
    else
        ui->tableView->setColumnWidth(0,250);
    ui->tableView->setColumnWidth(1,130);
    ui->tableView->setColumnWidth(2,150);
    //ui->tableView->setColumnWidth(3,130);
    client->clear();                            //清空消息队列
}

//加入房间按钮
void Menu::join_game()
{
    client->clear();
    QPushButton *button = (QPushButton *)sender();      //获取信号发送者对象(join按钮)
    QString msg="J"+button->property("Fd").toString();
    if(!client->isConnected())
            return;
    int ret=client->send_msg(msg);
    qDebug() << "send msg:" << msg << Qt::endl;
    Sleep(300);
    bool flag = (msg == "success");
    qDebug() << "ret:" << ret << " queue_empty:" << client->queue_empty() << " msg==success:" << flag << Qt::endl;
    if(ret==SOCKET_ERROR || client->queue_empty()|| client->get_msg()!="success")
    {
        QMessageBox::information(this,"网络对战","房间加入失败,请重试",QMessageBox::Ok);
        on_refresh_btn_clicked();
        return ;
    }
    client->clear();
    qDebug()<<"已加入房间,套接字:"<<button->property("Fd").toString()<<Qt::endl;
    tableModel->removeRows(0,tableModel->rowCount());
    internet_game *net_game=new internet_game(this->x(),this->y(),client,button->property("Name").toString());
    net_game->show();
    this->hide();
    connect(net_game,&internet_game::gameOver,this,[=](){
        delete net_game;
        on_refresh_btn_clicked();
        this->show();
    });
}

