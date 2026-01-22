#include "internet_game.h"
#include "ui_internet_game.h"

#define chessboard_size 15

//构造函数 初始化所有属性
internet_game::internet_game(int x,int y,client_net *client_,QString room_name,QWidget *parent) :
    client(client_),                    //初始化client网络通信对象
    QWidget(parent),
    ui(new Ui::internet_game)
{
    ui->setupUi(this);
    this->setFixedSize(1000,800);
    this->move(x,y);
    ui->Title->setText(room_name);          //展示房间名
    setWindowTitle("五子棋网络对战");
    setWindowIcon(QPixmap(":new/prefix1/img/Title1.png"));

    ui->stackedWidget->setCurrentIndex(0);
    ui->button_black->hide();
    ui->button_white->hide();
    ui->label_victory->hide();
    ui->Label_your_color->hide();
    ui->button_agree->hide();
    ui->button_refuse->hide();
    ui->label_wait_answer->hide();
    ui->label_anwser->hide();
    ui->label_msg->hide();

    white_chess.load(":/new/prefix1/img/shiro.png");
    black_chess.load(":/new/prefix1/img/kuro.png");
    board_bg.load(":/new/prefix1/img/menubg1.png");

    wait=false;
    ban_mouse=false;
    prepare=false;
    color=-1;
    running=false;
    turn=false;
    timerId1=startTimer(500);

    back.resize(0);             //记录棋盘信息初始化
    square = 800 / (chessboard_size + 1);           //格子边长赋值
    //保存每个点的信息  [BUG]下标必须从0开始
    for(int i = 0; i < chessboard_size; i++)
    {
        chess_info.push_back(QVector<QPair<QRect, int>>());
        for(int j = 0; j < chessboard_size; j++)
        {
            //设置每个点的点击范围
            chess_info[i].push_back(QPair<QRect, int>(QRect((i+1)*square - square * 1.25 / 3, (j+1)*square - square * 1.25 / 3, square/3*2.5, square/3*2.5), -1));
        }
    }
}
internet_game::internet_game(QWidget *parent)
    : QWidget(parent), ui(new Ui::internet_game) {
    ui->setupUi(this);
    // 这里初始化成员变量，设置默认值等
    // 因为没有提供足够的参数来初始化所有成员，所以可能需要设置默认值或预留接口
    client = nullptr; // 需要在外部初始化 client 对象
    white_chess.load(":/new/prefix1/img/shiro.png");
    black_chess.load(":/new/prefix1/img/kuro.png");
    board_bg.load(":/new/prefix1/img/menubg1.png");

    wait = false;
    ban_mouse = false;
    prepare = false;
    color = -1;
    running = false;
    turn = false;
    timerId1 = startTimer(500); // 启动定时器

    back.resize(0); // 记录棋盘信息初始化
    square = 800 / (chessboard_size + 1); // 格子边长赋值
    // 保存每个点的信息
    for (int i = 0; i < chessboard_size; i++) {
        chess_info.push_back(QVector<QPair<QRect, int>>());
        for (int j = 0; j < chessboard_size; j++) {
            // 设置每个点的点击范围
            chess_info[i].push_back(QPair<QRect, int>(
                QRect((i + 1) * square - square * 1.25 / 3, (j + 1) * square - square * 1.25 / 3, square / 3 * 2.5, square / 3 * 2.5), -1));
        }
    }
}

internet_game::~internet_game()
{
    qDebug() << "internet_game对象析构..." << Qt::endl;
    client->send_msg("E");              //(*)向服务器发送退出房间请求，更新服务器信息
    delete ui;
}

void internet_game::initialization()
{
    //prepare=false;
    ui->LE_recv->clear();           //清空聊天信息框
    back.resize(0);                 //清空栈
    for(auto &x:chess_info)
    {
        for(auto &y:x)
            y.second=-1;            //清空落子信息容器
    }
    update();                       //更新棋盘界面
}

void internet_game::take_chess(int x, int y)
{
    if(!running)        //游戏未开始时不能落子
        return;
    if(!turn)           //不是己方回合不能落子
        return;
    qDebug() << "落子位置:" << x << " " << y << Qt::endl;
    QRect r(x, y, 5, 5);        //点击的触摸区域
    //核心是判断是否点击在周围的格子，难点*
    for(int i = 0; i < chess_info.size(); i++)
    {
        for(int j = 0; j < chess_info[i].size(); j++)
        {
            //如果点击位置相交且该点没有落子
            if(chess_info[i][j].first.intersects(r) && chess_info[i][j].second == -1)
            {
                chess_info[i][j].second = color;        //记录该点落子颜色
                back.push(QPair<int, int>(i, j));
                qDebug() << i << " " << j;
                QString msg="OM00";
                switch(i)                           //存储x坐标的值
                {
                case 10: msg[2] = 'a'; break;       //如果是10则转换成字符a,以此类推
                case 11: msg[2] = 'b'; break;
                case 12: msg[2] = 'c'; break;
                case 13: msg[2] = 'd'; break;
                case 14: msg[2] = 'e'; break;
                default: msg[2] = QChar('0' + i); // 使用 QChar 构造函数
                }
                switch(j)                           //存储y坐标的值
                {
                case 10: msg[3] = 'a'; break;
                case 11: msg[3] = 'b'; break;
                case 12: msg[3] = 'c'; break;
                case 13: msg[3] = 'd'; break;
                case 14: msg[3] = 'e'; break;
                default: msg[3] = QChar('0' + j); // 使用 QChar 构造函数
                }
                client->send_msg(msg);
                win(i, j);                      //己方落子胜利判断与回合转换
            }
        }
    }
    update();
}

void internet_game::win(int x, int y)
{
    int dir1[4][2] = {
                        {0, 1},     //上下
                        {1, 1},     //左上右下
                        {1, 0},     //左右
                        {1, -1}     //右下左上
                     };
    int dir2[4][2] = {
                        {0, -1},    //上下
                        {-1, -1},   //左上右下
                        {-1, 0},    //左右
                        {-1, 1}     //右下左上
                     };
    bool black = (turn ? color : !color);           //判断当前落子方棋子颜色
    //枚举4对方向
    for(int i = 0; i < 4; i++)
    {
        int sum = 0;
        int a = x, b = y;
        //验证该方向上前4个点
        for(int j = 0; j <= 4; j++)
        {
            a += dir1[i][0];
            b += dir1[i][1];
            //[BUG] chess_info[a][b].second == black 表示该点上的棋子颜色与当前落子方的颜色一致
            if(a >= 0 && b >= 0 && a < chess_info.size() && b < chess_info.size() && chess_info[a][b].second == black)
                sum++;
            else
                break;
        }

        //[BUG] 还原a,b点
        a = x, b = y;

        //验证该方向上后4个点
        for(int j = 0; j <= 4; j++)
        {
            a += dir2[i][0];
            b += dir2[i][1];
            //[BUG] chess_info[a][b].second == black 表示该点上的棋子颜色与当前落子方的颜色一致
            if(a >= 0 && b >= 0 && a < chess_info.size() && b < chess_info.size() && chess_info[a][b].second == black)
                sum++;
            else
                break;
        }

        if(sum >= 4)                //五子相连 游戏结束
        {
            if(black)
                ui->label_victory->setText("黑方胜利");
            else
                ui->label_victory->setText("白方胜利");

            on_Button_prepare_clicked();    //(游戏结束)调用准备按钮函数 相当于取消准备
            ui->label_prepare->show();
            ui->label_victory->show();
            ui->stackedWidget->setCurrentIndex(0);
            color=-1;
            running=false;
            return ;
        }
    }
    turn = !turn;               //*交换回合
}


void internet_game::go_back()
{
    if(back.empty())
        return;
    int x = back.top().first;           //[BUG]top访问栈顶 不能写成pop
    int y = back.top().second;
    chess_info[x][y].second = -1;       //将该点落子记录清除
    back.pop();                         //栈顶元素出栈
    turn = !turn;                       //交换回合
    update();                           //重画棋盘
}

void internet_game::wait_over()
{
    wait = false;
    ban_mouse = false;
    ui->btn_back->setDisabled(false);
    ui->button_agree->hide();
    ui->button_refuse->hide();
    ui->label_victory->hide();
    ui->label_prepare->hide();
    ui->label_wait_answer->hide();
}

void internet_game::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPen pen;
    pen.setWidth(3);
    painter.setPen(pen);
    painter.drawPixmap(0, 0, width(), height(), board_bg);

    int value = chessboard_size + 1;
    int grid_width = 800 / value;
    int grid_height = 800 / value;
    int w = 0, h = 0;

    //绘制棋盘横竖15条线
    for(int i = 0; i < chessboard_size; i++)
    {
        w += grid_width;
        h += grid_height;
        //上下左右留出一个格子的空间
        painter.drawLine(w, grid_height, w, 800 - grid_height);
        painter.drawLine(grid_width, h, 800 - grid_width, h);
    }

    //绘制棋盘中心点
    pen.setWidth(8);
    painter.setPen(pen);
    painter.drawPoint(800 / 2, 800 / 2);

    //以上是绘制棋盘，接下来就是绘制棋子
    for(auto x:chess_info)
    {
        for(auto y:x)
        {
            //painter.drawRect(y.first);
            switch(y.second)
            {
                case 1:painter.drawPixmap(y.first,black_chess);break;
                case 0:painter.drawPixmap(y.first,white_chess);break;
                case -1:break;
                default:break;
            }

            if(!back.empty())
            {
                pen.setWidth(8);
                pen.setColor(QColor(Qt::red));
                painter.setPen(pen);
                painter.drawPoint((back.top().first + 1) * 50, (back.top().second + 1) * 50);
            }
        }
    }
    if(running)
        if(turn)            //如果是你的回合
        {
            ui->label_msg->setText("你的回合");
            ui->label_msg->setStyleSheet("QLabel{""color:green;""}");
        }
        else
        {
            ui->label_msg->setText("对手回合");
            ui->label_msg->setStyleSheet("QLabel{""color:red;""}");
        }
}

//获取双方准备信息
void internet_game::get_prepare_information()
{
    QString msg;
    client->clear();
    client->send_msg("U");          //发送获取对手准备信息请求
    while(client->queue_size()<4 && client->isConnected());     //如果没有收到响应消息则阻塞
    msg=client->get_msg();          //获取房间对手是否有对手
    msg=="1"?ui->label_opponent->setStyleSheet("QLabel{""border-image:url(:/new/prefix1/img/dog.png);""}"):
             ui->label_opponent->setStyleSheet("QLabel{""border-image:url(:/new/prefix1/img/none.png);""}");

    msg=client->get_msg();          //获取对手是否已准备
    msg=="1"?ui->label_prepare_->setText("已准备") : ui->label_prepare_->setText("未准备");
    msg=="1"?ui->label_prepare_->setStyleSheet("QLabel{""color:green;""}") : ui->label_prepare_->setStyleSheet("QLabel{""color:red;""}");

    msg=QString("IP:%1").arg(client->get_msg());        //获取对手IP
    ui->label_ip->setText(msg);
    msg=QString("FD:%1").arg(client->get_msg());        //获取对手套接字
    ui->label_fd->setText(msg);
    update();                       //更新视图
}

void internet_game::timerEvent(QTimerEvent *event)
{
    if(!client->isConnected())              //网络连接中断退出
    {
        QMessageBox::information(this,"Warnning","网络连接中断",QMessageBox::Ok);
        client->clear();
        close();
        return ;
    }
    if(!running)
    {
        ui->label_msg->hide();
        if(!client->queue_empty())
        {
            if(client->get_msg() == "start")      //双方已经准备就绪
            {
                initialization();
                ui->label_victory->hide();
                ui->stackedWidget->setCurrentIndex(1);
                ui->button_black->show();
                ui->button_white->show();
                ui->label_msg->show();
                ui->label_prepare->hide();
                ui->Label_your_color->hide();
                running=true;                      //正在运行
                //Sleep(300);
                return;                         //游戏已开始直接返回，不用再获取准备信息
            }
        }
        get_prepare_information();          //获取对手准备信息
    }
    else if(!wait)          //如果当前不处于等待状态
    {
        //根据服务器端发来的消息确定该客户端是先手或后手
        if(color == -1)             //color为-1,表示游戏未开始
        {
            while(!client->queue_empty())
            {
                QString msg = client->get_msg();
                qDebug() << "get_msg: " << msg << Qt::endl;
                //判断对手是否离开了游戏
                string str = msg.toStdString();
                if(str[0] == 'O' && str[1] == 'R')
                {
                    on_Button_prepare_clicked();    //(游戏结束)调用准备按钮函数，相当于取消准备
                    ui->label_prepare->show();
                    ui->label_victory->setText("对手退出了游戏");
                    ui->label_victory->show();
                    ui->button_black->hide();
                    ui->button_white->hide();
                    QTimer *timer = new QTimer(this);           //new一个定时器(5秒后发送timeout信号)
                    timer->start(5000);
                    connect(timer, &QTimer::timeout, this, [=](){
                        ui->label_victory->hide();
                        timer->stop();
                        delete timer;
                    });
                    ui->stackedWidget->setCurrentIndex(0);
                    color = -1;
                    running = false;
                }

                if(msg == "c1")         //如果是先手即黑方
                {
                    color = 1;
                    turn = true;        //*轮到己方回合
                    ui->button_black->hide();
                    ui->button_white->hide();
                    ui->Label_your_color->show();
                    ui->Label_your_color->setStyleSheet("QLabel{border-image:url(:/new/prefix1/img/kuro.png)}");
                }
                else if(msg == "c0")         //如果是先手即黑方
                {
                    color = 0;
                    turn = false;       //*不是己方回合
                    ui->button_black->hide();
                    ui->button_white->hide();
                    ui->Label_your_color->show();
                    ui->Label_your_color->setStyleSheet("QLabel{border-image:url(:/new/prefix1/img/shiro.png)}");
                }
            }
        }
        else                            //游戏开始后 处理服务器传来的消息
        {
            while(!client->queue_empty())
            {
                //处理对手落子后的信息(存储信息、回合交换、胜利判断)
                QString recv = client->get_msg();
                string msg = recv.toStdString();
                if(msg[0] == 'O')
                    switch(msg[1])
                    {
                        case 'M':           //对手落子处理
                        {
                            //处理获得落子x,y的坐标
                            int x, y;
                            if(msg[2] >= 'a' && msg[2] <= 'e')
                                x = msg[2] - 'a' + 10;
                            else
                                x = msg[2] - '0';
                            if(msg[3] >= 'a' && msg[3] <= 'e')
                                y = msg[3] - 'a' + 10;
                            else
                                y = msg[3] - '0';
                            chess_info[x][y].second = !color;           //存储对手的落子信息
                            back.push(QPair<int, int>(x, y));
                            update();           //更新棋盘
                            win(x, y);          //进行回合交换与胜利判断
                        }
                        break;
                        case 'R':           //对手退出房间处理
                        {
                            on_Button_prepare_clicked();    //(游戏结束)调用准备按钮函数，相当于取消准备
                            ui->label_prepare->show();
                            ui->label_victory->setText("对手退出了游戏");
                            ui->label_victory->show();
                            QTimer *timer = new QTimer(this);           //new一个定时器(5秒后发送timeout信号)
                            timer->start(5000);
                            connect(timer, &QTimer::timeout, this, [=](){
                                ui->label_victory->hide();
                                timer->stop();
                                delete timer;
                            });
                            ui->stackedWidget->setCurrentIndex(0);
                            color = -1;
                            running = false;
                        }
                        break;
                        case 'S':           //对手认输处理
                        {
                            on_Button_prepare_clicked();    //(游戏结束)调用准备按钮函数，相当于取消准备
                            ui->label_prepare->show();
                            ui->label_victory->setText("对手已认输");
                            ui->label_victory->show();
                            ui->stackedWidget->setCurrentIndex(0);
                            color=-1;
                            running=false;
                        }
                        break;
                        case 'N':           //对手的聊天消息
                        {
                            QString text = "[对手]:" + recv.mid(2);         //从下标为2的字符开始的字符串
                            ui->LE_recv->append(text);
                        }
                        break;
                        case 'B':           //对手的悔棋请求处理
                        {
                            ui->label_victory->setText("是否同意对手悔棋");
                            ui->label_victory->show();
                            ui->btn_back->setDisabled(true);
                            ui->button_agree->show();           //同意按钮
                            ui->button_refuse->show();          //拒绝按钮
                            ban_mouse = true;
                            return;
                        }
                        break;
                        default:
                            break;
                    }
            }
        }
    }
    else                    //等待状态下的处理 即等待悔棋请求响应
    {
        if(!client->queue_empty())          //如果收到对手的应答消息，同步本地客户端棋盘信息
        {
            string msg = client->get_msg().toStdString();
            if(msg[0] == 'O' && msg.size() == 3)
            switch(msg[1])
            {
                case 'B':
                {
                    QTimer *timer = new QTimer(this);
                    if(msg[2] == '1')
                    {
                        ui->label_anwser->setText("对手同意悔棋");
                        if(turn)
                        {
                           go_back();go_back();     //己方回合后退2步
                        }
                        else
                            go_back();              //对方回合后退1步
                    }
                    else
                    {
                        ui->label_anwser->setText("对手不同意悔棋");
                    }
                    ui->label_anwser->show();
                    connect(timer, &QTimer::timeout, this, [=](){
                       ui->label_anwser->hide();
                       timer->stop();
                       delete timer;
                    });
                    timer->start(3000);         //文字显示3秒(3秒后发送timeout信号)
                    wait_over();
                }
            }
        }
    }
}


void internet_game::mousePressEvent(QMouseEvent *event)
{
    if(!ban_mouse)
        take_chess(event->x(), event->y());         //检测到鼠标点击事件并落子
}

void internet_game::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return)              //键盘回车键触发点击发送按钮
        on_Button_send_clicked();
}

void internet_game::closeEvent(QCloseEvent *event)
{
    qDebug() << "网络游戏页面关闭..." << Qt::endl;
    if(running)                     //游戏运行中退出,通知对手已退出游戏
    {
        client->send_msg("OR");     //通过服务器将退出消息转发给对手
    }
    emit gameOver();                 //发送游戏结束信号，触发网络页面析构
}

void internet_game::on_Button_exit_room_clicked()
{
    close();
}

void internet_game::on_btn_exit_clicked()
{
    close();
}

//准备按钮点击
void internet_game::on_Button_prepare_clicked()
{
    int ret;
    //Sleep(500);
    ret = client->send_msg("prepare");      //发送准备请求
    if(ret==SOCKET_ERROR)
        return;

    //此时的prepare是点击前的状态
    else if(prepare)
    {
        ui->label_prepare->setText("请准备...");
        ui->label_prepare->setStyleSheet("QLabel{color:rgba(255,0,0,0.6);}");
    }
    else
    {
        ui->label_prepare->setText("等待对手准备...");
        ui->label_prepare->setStyleSheet("QLabel{color:rgba(0,255,0,0.6);}");
    }

    prepare=!prepare;                   //更新准备状态

    //更改按钮文字
    if(prepare)
    {
        ui->Button_prepare->setText("取消准备");
    }
    else
    {
        ui->Button_prepare->setText("准备");
    }
}

//先手按钮点击
void internet_game::on_button_black_clicked()
{
    if(color != -1)
        return;
    client->send_msg("color1");
}

//后手按钮点击
void internet_game::on_button_white_clicked()
{
    if(color != -1)
        return;
    client->send_msg("color0");
}

//认输按钮点击
void internet_game::on_btn_surrender_clicked()
{
    if(!running || color == -1)
        return;
    client->send_msg("OS");             //向服务器发送认输消息,通知对手已认输
    on_Button_prepare_clicked();
    ui->label_prepare->show();
    ui->label_victory->setText("你已认输");
    ui->label_victory->show();
    ui->stackedWidget->setCurrentIndex(0);
    color = -1;
    running = false;
}

//发送按钮点击
void internet_game::on_Button_send_clicked()
{
    if(ui->LE_send->text() == "")
        return;
    QString msg = "ON" + ui->LE_send->text();
    ui->LE_recv->append("[你]:" + ui->LE_send->text());
    ui->LE_send->clear();
    client->send_msg(msg);      //将消息发送给服务器
}

//悔棋按钮点击
void internet_game::on_btn_back_clicked()
{
    if(!running || color == -1 || back.empty())
        return;
    client->send_msg("OB");         //向服务器发送悔棋请求
    ui->label_wait_answer->setText("请等待对手做出回应");
    ui->label_wait_answer->show();
    ban_mouse = true;              //禁用鼠标
    wait = true;                   //等待状态
}

//同意悔棋按钮
void internet_game::on_button_agree_clicked()
{
    client->send_msg("OB1");         //向服务器发送悔棋同意信息
    if(!turn)               //如果不是己方回合 则向前退回两步
    {
        go_back();
        go_back();
    }
    else
        go_back();
    wait_over();
}

//拒绝悔棋按钮
void internet_game::on_button_refuse_clicked()
{
    client->send_msg("OB0");        //向服务器发送悔棋拒绝信息
    wait_over();
}
