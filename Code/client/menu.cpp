/**
 * @file menu.cpp
 * @brief 主菜单界面实现
 *
 * 本文件实现了五子棋游戏的主菜单功能，包括：
 * - 主界面初始化与UI管理
 * - 本地游戏入口
 * - 网络游戏入口（连接服务器、创建房间、加入房间）
 * - 房间列表的显示与刷新
 * - 网络连接状态的实时监控
 *
 * 作为整个客户端程序的导航中心，负责各个游戏模式的入口和窗口管理
 */

#include "menu.h"
#include "ui_menu.h"
#include "gamewin.h"

/**
 * @brief 连接线程函数的前向声明
 * @param arg 传入的client_net对象指针
 * @return unsigned 线程返回值
 *
 * 使用WINAPI调用约定，用于Windows多���程
 * 在独立线程中执行服务器连接操作，避免阻塞UI
 */
unsigned WINAPI connect_thread(void *arg);

/**
 * @brief 构造函数 - 初始化主菜单界面
 * @param parent 父窗口指针，默认为nullptr
 *
 * 初始化流程：
 * 1. 设置UI界面
 * 2. 配置窗口属性（大小、标题、图标）
 * 3. 初始化堆叠窗口（主菜单页/网络大厅页）
 * 4. 初始化房间列表表格模型
 * 5. 创建网络通信对象
 * 6. 设置表格列宽
 */
Menu::Menu(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Menu)
{
    // 初始化UI界面（由Qt Designer生成的ui_menu.h定义）
    ui->setupUi(this);

    // 设置窗口固定大小为600x600像素
    this->setFixedSize(600, 600);

    // 设置窗口标题和图标
    setWindowTitle("五子棋对战");
    setWindowIcon(QPixmap(":/new/prefix1/img/Title1.png"));

    // 设置堆叠��口背景（已注释）
    //ui->stackedWidget->setStyleSheet("border-image: url(:/new/prefix1/img/menubg1.png);");

    // 初始化显示第一页（主菜单页面，索引0）
    // stackedWidget包含两个页面：
    // 页面0: 主菜单（本地游戏、网络游戏、退出等按钮）
    // 页面1: 网络游戏大厅（房间列表、创建房间等）
    ui->stackedWidget->setCurrentIndex(0);

    // 初始化房间列表的表格模型
    // QStandardItemModel用于管理表格数据
    tableModel = new QStandardItemModel(this);
    ui->tableView->setModel(tableModel);

    // 设置表格列标题
    //columnTitle << "房间信息" << "IP地址" <<"套接字"<<"进入房间";
    columnTitle << "房间信息" << "IP地址" << "进入房间";
    tableModel->setHorizontalHeaderLabels(columnTitle);

    // 创建网络通信对象
    // client_net封装了与服务器的所有网络通信功能
    client = new client_net();              //初始化客户端网络

    // 设置表格各列的宽度（单位：像素）
    ui->tableView->setColumnWidth(0,265);   // 房间信息列
    ui->tableView->setColumnWidth(1,130);   // IP地址列
    //ui->tableView->setColumnWidth(2,50);  // 套接字列（已注释）
    ui->tableView->setColumnWidth(2,150);   // 进入房间按钮列

}

/**
 * @brief 析构函数 - 清理资源
 *
 * 释放动态分配的内存：
 * 1. 网络通信对象
 * 2. 表格数据模型
 * 3. UI对象
 */
Menu::~Menu()
{
    qDebug() << "主页面析构~" <<Qt::endl;
    delete client;          //释放网络信息传输对象内存
    delete tableModel;      // 释放表格模型内存
    delete ui;              // 释放UI内存

}

/**
 * @brief 定时器事件处理 - 监控网络连接状态
 * @param event 定时器事件（未使用）
 *
 * 定时器每500ms触发一次，用于实时更新连接状态显示：
 * - 正在连接：蓝色文字 "<正在连接服务器...>"
 * - 已连接：绿色文字 "已连接服务器-<创建或加入对局>"，激活功能按钮
 * - 未连接：红色文字 "<请连接服务器>"，禁用功能按钮
 *
 * 这种轮询方式确保UI能够实时反映网络状态变化
 */
void Menu::timerEvent(QTimerEvent *event)
{
    // 状态1: 正在连接中
    if(client->connect_thread_running)      //正在连接
    {
        // 显示蓝色提示文字
        ui->connect_stat_label->setStyleSheet("QLabel{color:blue}");
        ui->connect_stat_label->setText("<正在连接服务器...>");
        return;
    }
    // 状态2: 已成功连接
    else if(client->isConnected())          //已经连接
    {
        // 激活创建房间和刷新房间列表按钮
        ui->create_btn->setDisabled(false);     //激活创建对局按钮
        ui->refresh_btn->setDisabled(false);    //激活刷新对局按钮

        // 显示绿色提示文字
        ui->connect_stat_label->setStyleSheet("QLabel{color:green}");
        ui->connect_stat_label->setText("已连接服务器-<创建或加入对局>");
        return;
    }
    // 状态3: 未连接
    else                                    //没连接
    {
        // 禁用创建房间和刷新房间列表按钮
        ui->create_btn->setDisabled(true);          //[BUG] setDisable不是setDefault
        ui->refresh_btn->setDisabled(true);

        // 显示红色提示文字
        ui->connect_stat_label->setStyleSheet("QLabel{color:red}");
        ui->connect_stat_label->setText("<请连接服务器>");
        return;
    }
}

/**
 * @brief 本地游戏按钮点击事件处理
 *
 * 点击流程：
 * 1. 创建本地游戏窗口（GameWin）
 * 2. 显示游戏窗口
 * 3. 隐藏主菜单窗口
 * 4. 连接游戏结束信号，用于窗口切换和资源释放
 *
 * 窗口管理采用"隐藏-显示"模式：
 * - 进入游戏时隐藏菜单、显示游戏窗口
 * - 退出游戏时删除游戏窗口、显示菜单
 */
//本地游戏按钮点击事件
void Menu::on_local_game_btn_clicked()
{
    // 动态创建本地游戏窗口
    GameWin *local_game = new GameWin();

    // 显示游戏窗口
    local_game->show();         //展示本地游戏主界面

    // 隐藏主菜单（而非关闭，以便稍后恢复）
    this->hide();               //隐藏菜单界面

    // 连接游戏结束信号到Lambda表达式
    // 当GameWin发出gameOver信号时执行以下操作
    //游戏结束信号发出后显示主菜单并释放空间
    connect(local_game, &GameWin::gameOver, [=](){
        this->show();           // 重新显示主菜单
        delete local_game;      // 释放游戏窗口内存
    });
}


/**
 * @brief 关于按钮点击事件处理
 *
 * 显示项目信息的消息框
 */
void Menu::on_about_btn_clicked()
{
    // 显示项目作者信息
    QMessageBox::information(this, "作者", "制作人：清兵卫@github \n本项目开源供大家学习参考", QMessageBox::Ok);
}

/**
 * @brief 退出游戏按钮点击事件处理
 *
 * 关闭主窗口，结束程序
 */
//退出游戏按钮点击事件
void Menu::on_exit_btn_clicked()
{
    close();    // 关闭窗口，触发程序退出
}

/**
 * @brief 网络游戏按钮点击事件处理
 *
 * 点击流程：
 * 1. 启动定时器（用于监控连接状态）
 * 2. 自动尝试连接服务器
 * 3. 切换到网络游戏大厅页面
 * 4. 禁用功能按钮（等待连接成功后由定时器激活）
 */
//网络游戏按钮
void Menu::on_net_game_btn_clicked()
{
    // 启动定时器，每500ms触发timerEvent
    // 用于实时监控和显示连接状态
    timerId = startTimer(500);              //启动定时器（500ms调用一次timerEvent）

    // 自动开始连接服务器
    on_reconnect_btn_clicked();             //进入页面首先连接服务器

    // 切换到网络游戏大厅页面（stackedWidget的第2页，索引1）
    ui->stackedWidget->setCurrentIndex(1);

    // 初始禁用功能按钮，等待连接成功后由timerEvent激活
    ui->create_btn->setDisabled(true);
    ui->refresh_btn->setDisabled(true);
}

/**
 * @brief 返回主页面按钮点击事件处理
 *
 * 退出网络大厅，返回主菜单：
 * 1. 清空房间列表
 * 2. 停止状态监控定时器
 * 3. 终止连接线程（如果正在运行）
 * 4. 断开网络连接（如果已连接）
 * 5. 切换回主菜单页面
 */
//返回主页面
void Menu::on_exit_net_btn_clicked()
{
    // 清空房间列表表格
    tableModel->removeRows(0, tableModel->rowCount());       //清空房间列表

    // 停止连接状态监控定时器
    killTimer(timerId);

    // 如果连接线程正在运行，设置标志使其退出
    if(client->connect_thread_running)
        client->connect_thread_running = false;

    // 如果已连接，断开与服务器的连接
    if(client->isConnected())
        client->disconnect();

    // 切换回主菜单页面（索引0）
    ui->stackedWidget->setCurrentIndex(0);              //切回主页面
}

/**
 * @brief 连接服务器按钮点击事件处理
 *
 * 在独立线程中尝试连接服务器：
 * - 如果已连接，直接返回
 * - 否则创建新线程执行连接操作
 *
 * 使用多线程避免连接过程阻塞UI
 */
//连接服务器按钮功能
void Menu::on_reconnect_btn_clicked()
{
    // 已连接则无需重复连接
    if(client->isConnected())
        return;

    // 创建连接线程
    // _beginthreadex: Windows多线程函数
    // 参数: 安全属性、栈大小、线程函数、参数、创建标志、线程ID指针
    //开启连接线程
    HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, connect_thread, (void*)client, 0, NULL);
}

/**
 * @brief 连接线程函数
 * @param arg client_net对象指针
 * @return unsigned 线程返回值（未使用）
 *
 * 在独立线程中执行服务器连接操作
 * 连接结果通过client对象的状态属性反映
 * UI通过定时器轮询获取连接状态
 */
//连接线程函数
unsigned WINAPI connect_thread(void *arg)
{
    // 将void*参数转换为client_net指针
    client_net *clnt = (client_net*)arg;

    qDebug() << "连接线程开始..." << Qt::endl;

    // 调用connect方法尝试连接服务器
    // 该方法内部会进行多次重试
    clnt->connect();

    qDebug() << "连接线程结束..." <<Qt::endl;

    // 注意：这里没有return语句，应该return 0
}

/**
 * @brief 创建房间按钮点击事件处理
 *
 * 创建房间流程：
 * 1. 检查网络连接状态
 * 2. 构造创建房间消息并发送给服务器
 * 3. 检查发送是否成功
 * 4. 创建网络对战游戏窗口
 * 5. 显示游戏窗口，隐藏菜单
 * 6. 连接游戏结束信号
 *
 * 消息协议：
 * - 格式: "C:房间名"
 * - C表示Create（创建）
 */
//创建房间按钮功能实现
void Menu::on_create_btn_clicked()
{
    // 未连接时不能创建房间
    if(!client->isConnected())
        return;

    // 构造创建房间的消息
    // 格式: "C:" + 房间名（从输入框获取）
    QString create_str = "C:" + ui->LineEdit->text();       //房间名

    // 发送创建房间请求
    int ret = client->send_msg(create_str);

    // 检查发送是否成功
    if(ret == SOCKET_ERROR)
    {
        QMessageBox::information(this, "网络对战", "创建失败,请重试", QMessageBox::Ok);
        return;
    }

    // 清空消息队列，准备接收新消息
    client->clear();

    // 创建网络对战游戏窗口
    // 参数: 窗口位置(继承自菜单)、网络对象、房间名
    //将client对象通过实参传入
    internet_game *inter_game = new internet_game(this->x(),this->y(),client,ui->LineEdit->text());

    // 显示游戏窗口，隐藏菜单
    inter_game->show();                 //进入网络对战棋盘
    this->hide();

    // 连接游戏结束信号
    // 当游戏窗口关闭时，释放资源并显示菜单
    //绑定游戏结束事件
    connect(inter_game, &internet_game::gameOver, [=](){
        qDebug() << "网络游戏结束" << Qt::endl;
        delete inter_game;      // 释放游戏窗口
        this->show();           //显示菜单
    });
}

/**
 * @brief 刷新房间列表按钮点击事件处理
 *
 * 刷新流程：
 * 1. 清空消息队列
 * 2. 向服务器发送刷新请求"R"
 * 3. 等待服务器响应
 * 4. 解析响应数据（在线人数、房间列表）
 * 5. 清空并重建表格显示
 * 6. 为每个房间创建"开战"按钮并绑定事件
 *
 * 服务器响应数据格式：
 * 1. 在线人数（字符串）
 * 2. 空闲房间数量（字符串）
 * 3. 对于每个房间：房间名、IP地址、套接字FD
 */
//刷新战局按钮
void Menu::on_refresh_btn_clicked()
{
    // 清空消息队列，避免旧消息干扰
    client->clear();

    // 未连接时不能刷新
    if(!client->isConnected())
        return ;

    // 发送刷新请求
    client->send_msg("R");                          //给服务器发送刷新请求

    // 等待服务器响应（阻塞300ms）
    Sleep(300);

    // 获取并显示在线人数
    int people=client->get_msg().toInt();           //在消息队列中获取已连接的客户端数
    ui->people_label->setText(QString("在线人数:%1").arg(people));

    // 获取空闲房间数量
    string str=client->get_msg().toStdString();        //获取没有人加入的空闲房间
    int sum=0;
    // 将字符串转换为数字
    for(int i=0;i<str.size();i++)
        sum=sum*10+str[i]-'0';                              //字符串转数字

    // 清空现有的房间列表
    tableModel->removeRows(0,tableModel->rowCount());       //先清空列表(初始化)

    // 遍历并显示每个空闲房间的信息
    for(int i=0;i<sum;i++)                          //遍历列表显示每个空闲房间信息
    {
        QString get;

        // 为每个房间创建"开战"按钮
        QPushButton *button = new QPushButton("开战");

        // 获取并显示房间名
        get=client->get_msg();
        tableModel->setItem(i, 0, new QStandardItem(get));  //显示房间名
        // 将房间名存储到按钮的自定义属性中，供join_game使用
        button->setProperty("Name",get);        //设置属性

        // 获取并显示房间创建者的IP地址
        get=client->get_msg();
        tableModel->setItem(i, 1, new QStandardItem(get));  //显示IP地址
        button->setProperty("Ip",get);

        // 获取房间创建者的套接字FD（不显示，但存储供加入时使用）
        get=client->get_msg();
        //tableModel->setItem(i, 2, new QStandardItem(get));  //显示套接字
        button->setProperty("Fd",get);

        // 将按钮放置到表格的第3列
        ui->tableView->setIndexWidget(tableModel->index(i, 2), button);

        // 绑定按钮点击事件到join_game函数
        connect(button,&QPushButton::pressed,this,&Menu::join_game);    //绑定join点击事件
    }

    // 根据是否有房间调整列宽
    if(sum==0)
        ui->tableView->setColumnWidth(0,265);
    else
        ui->tableView->setColumnWidth(0,250);
    ui->tableView->setColumnWidth(1,130);
    ui->tableView->setColumnWidth(2,150);
    //ui->tableView->setColumnWidth(3,130);

    // 清空消息队列
    client->clear();                            //清空消息队列
}

/**
 * @brief 加入房间功能实现
 *
 * 由房间列表中的"开战"按钮触发
 *
 * 加入流程：
 * 1. 清空消息队列
 * 2. 获取触发按钮的属性（房间信息）
 * 3. 构造加入房间消息并发送
 * 4. 等待服务器响应
 * 5. 检查加入是否成功
 * 6. 成功则创建游戏窗口，失败则提示并刷新列表
 *
 * 消息协议：
 * - 发送: "J" + 房间FD（加入请求）
 * - 接收: "success" 或 失败消息
 */
//加入房间按钮
void Menu::join_game()
{
    // 清空消息队列
    client->clear();

    // 获取信号发送者（被点击的按钮）
    // sender()返回发出信号的对象指针
    QPushButton *button = (QPushButton *)sender();      //获取信号发送者对象(join按钮)

    // 构造加入房间的消息
    // 格式: "J" + 目标房间的套接字FD
    QString msg="J"+button->property("Fd").toString();

    // 未连接时不能加入房间
    if(!client->isConnected())
        return;

    // 发送加入请求
    int ret=client->send_msg(msg);
    qDebug() << "send msg:" << msg << Qt::endl;

    // 等待服务器响应
    Sleep(300);

    // 调试信息
    bool flag = (msg == "success");
    qDebug() << "ret:" << ret << " queue_empty:" << client->queue_empty() << " msg==success:" << flag << Qt::endl;

    // 检查加入是否成功
    // 失败条件：发送失败、无响应、响应不是"success"
    if(ret==SOCKET_ERROR || client->queue_empty()|| client->get_msg()!="success")
    {
        // 显示失败提示
        QMessageBox::information(this,"网络对战","房间加入失败,请重试",QMessageBox::Ok);
        // 刷新房间列表（可能房间已被其他人加入）
        on_refresh_btn_clicked();
        return ;
    }

    // 加入成功，清空消息队列
    client->clear();
    qDebug()<<"已加入房间,套接字:"<<button->property("Fd").toString()<<Qt::endl;

    // 清空房间列表
    tableModel->removeRows(0,tableModel->rowCount());

    // 创建网络对战游戏窗口
    // 使用按钮属性中存储的房间名
    internet_game *net_game=new internet_game(this->x(),this->y(),client,button->property("Name").toString());

    // 显示游戏窗口，隐藏菜单
    net_game->show();
    this->hide();

    // 连接游戏结束信号
    connect(net_game,&internet_game::gameOver,this,[=](){
        delete net_game;            // 释放游戏窗口
        on_refresh_btn_clicked();   // 刷新房间列表
        this->show();               // 显示菜单
    });
}
