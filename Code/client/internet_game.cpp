/**
 * @file internet_game.cpp
 * @brief 网络对战游戏界面实现
 *
 * 本文件实现了五子棋网络对战的核心功能，包括：
 * - 游戏界面的初始化与绘制
 * - 网络通信与消息处理
 * - 落子逻辑与胜负判断
 * - 准备、认输、悔棋等游戏功能
 * - 实时聊天功能
 *
 * 使用Qt的信号槽机制处理用户交互
 * 使用定时器轮询服务器消息实现实时同步
 */

#include "internet_game.h"
#include "ui_internet_game.h"

/**
 * @brief 棋盘大小常量定义
 *
 * 五子棋标准棋盘为15x15的网格
 * 横竖各15条线，共225个交叉点可落子
 */
#define chessboard_size 15

/**
 * @brief 带参数的构造函数 - 初始化网络对战游戏界面
 * @param x 窗口显示的x坐标（继承自父窗口位置）
 * @param y 窗口显示的y坐标（继承自父窗口位置）
 * @param client_ 网络通信对象指针，用于与服务器通信
 * @param room_name 房间名称，显示在界面标题处
 * @param parent 父窗口指针，默认为nullptr
 *
 * 初始化流程：
 * 1. 设置窗口属性（大小、位置、标题、图标）
 * 2. 初始化UI组件状态（隐藏/显示）
 * 3. 加载棋子和棋盘图片资源
 * 4. 初始化游戏状态变量
 * 5. 启动定时器用于轮询服务器消息
 * 6. 初始化棋盘数据结构
 */
//构造函数 初始化所有属性
internet_game::internet_game(int x,int y,client_net *client_,QString room_name,QWidget *parent) :
    client(client_),                    //初始化client网络通信对象
    QWidget(parent),
    ui(new Ui::internet_game)
{
    // 初始化UI界面
    ui->setupUi(this);

    // 设置窗口固定大小为1000x800像素
    // 左侧800x800为棋盘区域，右侧200x800为控制面板
    this->setFixedSize(1000,800);

    // 将窗口移动到指定位置（继承父窗口的位置，保持视觉连贯）
    this->move(x,y);

    // 设置房间名称显示
    ui->Title->setText(room_name);          //展示房间名

    // 设置窗口标题和图标
    setWindowTitle("五子棋网络对战");
    setWindowIcon(QPixmap(":new/prefix1/img/Title1.png"));

    // 初始化堆叠窗口，显示第一页（准备界面）
    // stackedWidget包含两个页面：
    // 页面0: 准备界面（等待双方准备）
    // 页面1: 游戏界面（对战进行中）
    ui->stackedWidget->setCurrentIndex(0);

    // 隐藏游戏进行中才需要显示的UI组件
    ui->button_black->hide();       // 选择黑棋按钮
    ui->button_white->hide();       // 选择白棋按钮
    ui->label_victory->hide();      // 胜利提示标签
    ui->Label_your_color->hide();   // 己方棋子颜色显示
    ui->button_agree->hide();       // 同意悔棋按钮
    ui->button_refuse->hide();      // 拒绝悔棋按钮
    ui->label_wait_answer->hide();  // 等待对方回应提示
    ui->label_anwser->hide();       // 对方回应结果提示
    ui->label_msg->hide();          // 回合提示消息

    // 加载棋子和棋盘的图片资源
    white_chess.load(":/new/prefix1/img/shiro.png");    // 白棋图片
    black_chess.load(":/new/prefix1/img/kuro.png");     // 黑棋图片
    board_bg.load(":/new/prefix1/img/menubg1.png");     // 棋盘背景图片

    // 初始化游戏状态变量
    wait=false;         // 是否处于等待对方响应的状态（如等待悔棋回复）
    ban_mouse=false;    // 是否禁用鼠标点击（等待状态时禁用）
    prepare=false;      // 己方是否已准备
    color=-1;           // 己方棋子颜色（-1:未确定, 0:白棋, 1:黑棋）
    running=false;      // 游戏是否正在进行
    turn=false;         // 是否轮到己方落子

    // 启动定时器，每500毫秒触发一次timerEvent
    // 用于轮询服务器消息，实现实时同步
    timerId1=startTimer(500);

    // 初始化棋盘数据结构
    back.resize(0);             //记录棋盘信息初始化 清空落子历史栈

    // 计算每个格子的边长
    // 棋盘区域800像素，分成16份（15条线+两边各留半格）
    square = 800 / (chessboard_size + 1);           //格子边长赋值

    // 初始化棋盘每个交叉点的信息
    //保存每个点的信息  [BUG]下标必须从0开始
    for(int i = 0; i < chessboard_size; i++)
    {
        // 为每一列创建一个向量
        chess_info.push_back(QVector<QPair<QRect, int>>());
        for(int j = 0; j < chessboard_size; j++)
        {
            // 为每个交叉点设置点击检测区域和初始状态
            // QRect: 定义该点的可点击矩形区域
            // int: 该点的落子状态（-1:空, 0:白棋, 1:黑棋）
            //设置每个点的点击范围
            // 点击范围略大于格子，提升用户体验
            chess_info[i].push_back(QPair<QRect, int>(QRect((i+1)*square - square * 1.25 / 3, (j+1)*square - square * 1.25 / 3, square/3*2.5, square/3*2.5), -1));
        }
    }
}

/**
 * @brief 无参数的默认构造函数
 * @param parent 父窗口指针
 *
 * 用于创建未初始化网络连接的游戏对象
 * 需要在外部手动设置client对象
 *
 * 注意：这个构造函数主要用于Qt Designer预览或特殊场景
 */
internet_game::internet_game(QWidget *parent)
    : QWidget(parent), ui(new Ui::internet_game) {
    ui->setupUi(this);
    // 这里初始化成员变量，设置默认值等
    // 因为没有提供足够的参数来初始化所有成员，所以可能需要设置默认值或预留接口
    client = nullptr; // 需要在外部初始化 client 对象

    // 加载图片资源
    white_chess.load(":/new/prefix1/img/shiro.png");
    black_chess.load(":/new/prefix1/img/kuro.png");
    board_bg.load(":/new/prefix1/img/menubg1.png");

    // 初始化状态变量
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

/**
 * @brief 析构函数 - 清理资源并通知服务器退出房间
 *
 * 析构时自动向服务器发送退出房间请求"E"
 * 服务器收到后会更新房间状态并通知对手
 */
internet_game::~internet_game()
{
    qDebug() << "internet_game对象析构..." << Qt::endl;
    client->send_msg("E");              //(*)向服务器发送退出房间请求，更新服务器信息
    delete ui;
}

/**
 * @brief 初始化/重置棋盘状态
 *
 * 用于新游戏开始时重置所有棋盘数据：
 * 1. 清空聊天记录
 * 2. 清空落子历史栈
 * 3. 重置所有交叉点为空状态
 * 4. 刷新界面显示
 */
void internet_game::initialization()
{
    //prepare=false;
    ui->LE_recv->clear();           //清空聊天信息框
    back.resize(0);                 //清空栈（落子历史记录）

    // 遍历棋盘所有交叉点，重置为空状态
    for(auto &x:chess_info)
    {
        for(auto &y:x)
            y.second=-1;            //清空落子信息容器 -1表示该位置为空
    }
    update();                       //更新棋盘界面 触发paintEvent重绘
}

/**
 * @brief 落子处理函数
 * @param x 鼠标点击的x坐标（像素）
 * @param y 鼠标点击的y坐标（像素）
 *
 * 落子流程：
 * 1. 检查游戏状态和回合
 * 2. 判断点击位置是否在有效交叉点附近
 * 3. 检查该位置是否已有棋子
 * 4. 记录落子信息并发送给服务器
 * 5. 进行胜负判断
 *
 * 坐标编码规则：
 * - 0-9: 直接用字符'0'-'9'表示
 * - 10-14: 用字符'a'-'e'表示
 * - 消息格式: "OMxy" (O:对战消息, M:落子, x:列, y:行)
 */
void internet_game::take_chess(int x, int y)
{
    // 游戏未开始时不能落子
    if(!running)        //游戏未开始时不能落子
        return;

    // 不是己方回合不能落子
    if(!turn)           //不是己方回合不能落子
        return;

    qDebug() << "落子位置:" << x << " " << y << Qt::endl;

    // 创建一个小矩形作为点击检测区域
    QRect r(x, y, 5, 5);        //点击的触摸区域

    // 遍历棋盘所有交叉点，检测点击位置
    //核心是判断是否点击在周围的格子，难点*
    for(int i = 0; i < chess_info.size(); i++)
    {
        for(int j = 0; j < chess_info[i].size(); j++)
        {
            // 判断条件：点击区域与交叉点区域相交 且 该点为空
            //如果点击位置相交且该点没有落子
            if(chess_info[i][j].first.intersects(r) && chess_info[i][j].second == -1)
            {
                // 记录落子：将该点状态设为己方颜色
                chess_info[i][j].second = color;        //记录该点落子颜色

                // ��落子位置压入历史栈（用于悔棋功能）
                back.push(QPair<int, int>(i, j));
                qDebug() << i << " " << j;

                // 构造发送给服务器的落子消息
                // 格式: "OMxy" 其中x,y为坐标
                QString msg="OM00";

                // 编码x坐标（列）
                switch(i)                           //存储x坐标的值
                {
                case 10: msg[2] = 'a'; break;       //如果是10则转换成字符a,以此类推
                case 11: msg[2] = 'b'; break;
                case 12: msg[2] = 'c'; break;
                case 13: msg[2] = 'd'; break;
                case 14: msg[2] = 'e'; break;
                default: msg[2] = QChar('0' + i); // 使用 QChar 构造函数 0-9直接转字符
                }

                // 编码y坐标（行）
                switch(j)                           //存储y坐标的值
                {
                case 10: msg[3] = 'a'; break;
                case 11: msg[3] = 'b'; break;
                case 12: msg[3] = 'c'; break;
                case 13: msg[3] = 'd'; break;
                case 14: msg[3] = 'e'; break;
                default: msg[3] = QChar('0' + j); // 使用 QChar 构造函数
                }

                // 发送落子消息给服务器（服务器会转发给对手）
                client->send_msg(msg);

                // 进行胜负判断，如果未分胜负则交换回合
                win(i, j);                      //己方落子胜利判断与回合转换
            }
        }
    }

    // 刷新界面显示
    update();
}

/**
 * @brief 胜负判断函数
 * @param x 最后落子的x坐标（棋盘坐标，非像素）
 * @param y 最后落子的y坐标（棋盘坐标，非像素）
 *
 * 五子棋胜利条件：在横、竖、斜四个方向上有连续5个同色棋子
 *
 * 算法思路：
 * 1. 从落子点出发，检查4个方向（横、竖、左斜、右斜）
 * 2. 每个方向分正反两个向量，统计连续同色棋子数
 * 3. 如果任一方向连续同色棋子>=5，则该方获胜
 * 4. 未分胜负则交换回合
 */
void internet_game::win(int x, int y)
{
    // 定义4个方向的正向向量
    int dir1[4][2] = {
        {0, 1},     //上下（向下）
        {1, 1},     //左上右下（向右下）
        {1, 0},     //左右（向右）
        {1, -1}     //右下左上（向右上）
    };

    // 定义4个方向的反向向量
    int dir2[4][2] = {
        {0, -1},    //上下（向上）
        {-1, -1},   //左上右下（向左上）
        {-1, 0},    //左右（向左）
        {-1, 1}     //右下左上（向左下）
    };

    // 判断当前落子方的颜色
    // turn为true表示己方刚落子，颜色为color
    // turn为false表示对方刚落子，���色为!color
    bool black = (turn ? color : !color);           //判断当前落子方棋子颜色

    //枚举4对方向
    for(int i = 0; i < 4; i++)
    {
        int sum = 0;        // 该方向连续同色棋子计数（不包含当前落子点）
        int a = x, b = y;   // 从落子点开始检测

        // 向正方向检测，最多检测4个点
        //验证该方向上前4个点
        for(int j = 0; j <= 4; j++)
        {
            a += dir1[i][0];
            b += dir1[i][1];
            // 边界检查 + 颜色匹配检查
            //[BUG] chess_info[a][b].second == black 表示该点上的棋子颜色与当前落子方的颜色一致
            if(a >= 0 && b >= 0 && a < chess_info.size() && b < chess_info.size() && chess_info[a][b].second == black)
                sum++;
            else
                break;      // 遇到边界或不同颜色棋子，停止检测
        }

        // 重置坐标，准备检测反方向
        //[BUG] 还原a,b点
        a = x, b = y;

        // 向反方向检测，最多检测4个点
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

        // 加上落子点本身，如果>=5则获胜
        // sum统计的是除落子点外的连续同色棋子，所以>=4即表示总共>=5
        if(sum >= 4)                //五子相连 游戏结束
        {
            // 显示胜利信息
            if(black)
                ui->label_victory->setText("黑方胜利");
            else
                ui->label_victory->setText("白方胜利");

            // 游戏结束处理：取消准备状态，显示结果
            on_Button_prepare_clicked();    //(游戏结束)调用准备按钮函数 相当于取消准备
            ui->label_prepare->show();
            ui->label_victory->show();

            // 切换回准备界面
            ui->stackedWidget->setCurrentIndex(0);

            // 重置游戏状态
            color=-1;
            running=false;
            return ;
        }
    }

    // 未分胜负，交换回合
    turn = !turn;               //*交换回合
}


/**
 * @brief 悔棋操作 - 撤销最后一步落子
 *
 * 悔棋流程：
 * 1. 从历史栈中取出最后落子位置
 * 2. 将该位置重置为空状态
 * 3. 交换回合（因为撤销了一步）
 * 4. 刷新界面
 */
void internet_game::go_back()
{
    // 栈为空时无法悔棋
    if(back.empty())
        return;

    // 获取栈顶元素（最后一步落子位置）
    int x = back.top().first;           //[BUG]top访问栈顶 不能写成pop
    int y = back.top().second;

    // 清除该位置的落子记录
    chess_info[x][y].second = -1;       //将该点落子记录清除

    // 弹出栈顶元素
    back.pop();                         //栈顶元素出栈

    // 悔棋后回合也要还原
    turn = !turn;                       //交换回合

    // 刷新棋盘显示
    update();                           //重画棋盘
}

/**
 * @brief 结束等待状态
 *
 * 当收到对方的悔棋响应后调用，恢复正常游戏状态：
 * 1. 重置等待标志
 * 2. 恢复鼠标操作
 * 3. 恢复悔棋按钮
 * 4. 隐藏相关提示UI
 */
void internet_game::wait_over()
{
    wait = false;                       // 退出等待状态
    ban_mouse = false;                  // 恢复鼠标操作
    ui->btn_back->setDisabled(false);   // 恢复悔棋按钮

    // 隐藏等待相关的UI组件
    ui->button_agree->hide();
    ui->button_refuse->hide();
    ui->label_victory->hide();
    ui->label_prepare->hide();
    ui->label_wait_answer->hide();
}

/**
 * @brief 绘制事件处理 - 绘制棋盘和棋子
 * @param 未使用
 *
 * 绘制内容：
 * 1. 棋盘背景图
 * 2. 15x15的网格线
 * 3. 棋盘中心点（天元）
 * 4. 所有已落子的棋子
 * 5. 最后落子位置的红点标记
 * 6. 当前回合提示
 */
void internet_game::paintEvent(QPaintEvent *)
{
    // 创建画笔对象
    QPainter painter(this);
    QPen pen;
    pen.setWidth(3);        // 设置线宽为3像素
    painter.setPen(pen);

    // 绘制棋盘背景图，铺满��个窗口
    painter.drawPixmap(0, 0, width(), height(), board_bg);

    // 计算网格参数
    int value = chessboard_size + 1;        // 16（15条线+边距）
    int grid_width = 800 / value;           // 每格宽度（约50像素）
    int grid_height = 800 / value;          // 每格高度
    int w = 0, h = 0;                       // 当前绘制位置

    // 绘制15x15的网格线
    //绘制棋盘横竖15条线
    for(int i = 0; i < chessboard_size; i++)
    {
        w += grid_width;
        h += grid_height;
        // 绘制竖线（从上边距到下边距）
        //上下左右留出一个格子的空间
        painter.drawLine(w, grid_height, w, 800 - grid_height);
        // 绘制横线（从左边距到右边距）
        painter.drawLine(grid_width, h, 800 - grid_width, h);
    }

    // 绘制棋盘中心点（天元）
    //绘制棋盘中心点
    pen.setWidth(8);        // 加粗点的大小
    painter.setPen(pen);
    painter.drawPoint(800 / 2, 800 / 2);    // 在棋盘正中心绘制一个点

    // 绘制所有已落子的棋子
    //以上是绘制棋盘，接下来就是绘制棋子
    for(auto x:chess_info)
    {
        for(auto y:x)
        {
            //painter.drawRect(y.first);    // 调试用：绘制点击区域

            // 根据落子状态绘制对应颜色的棋子
            switch(y.second)
            {
            case 1:painter.drawPixmap(y.first,black_chess);break;   // 黑棋
            case 0:painter.drawPixmap(y.first,white_chess);break;   // 白棋
            case -1:break;                                          // 空位
            default:break;
            }

            // 在最后落子位置绘制红点标记
            if(!back.empty())
            {
                pen.setWidth(8);
                pen.setColor(QColor(Qt::red));      // 红色标记
                painter.setPen(pen);
                // 计算最后落子的像素位置并绘制红点
                painter.drawPoint((back.top().first + 1) * 50, (back.top().second + 1) * 50);
            }
        }
    }

    // 显示当前回合提示
    if(running)
        if(turn)            //如果是你的回合
        {
            ui->label_msg->setText("你的回合");
            ui->label_msg->setStyleSheet("QLabel{""color:green;""}");   // 绿色表示可以落子
        }
        else
        {
            ui->label_msg->setText("对手回合");
            ui->label_msg->setStyleSheet("QLabel{""color:red;""}");     // 红色表示等待
        }
}

/**
 * @brief 获取双方准备信息
 *
 * 向服务器请求房间信息，更新UI显示：
 * 1. 对手是否在房间
 * 2. 对手是否已准备
 * 3. 对手的IP地址
 * 4. 对手的套接字FD
 *
 * 通信协议：
 * - 发送: "U" (Update请求)
 * - 接收: 4条消息（是否有对手、准备状态、IP、FD）
 */
//获取双方准备信息
void internet_game::get_prepare_information()
{
    QString msg;
    client->clear();                // 清空消息队列，准备接收新消息
    client->send_msg("U");          //发送获取对手准备信息请求

    // 阻塞等待服务器返回4条消息
    while(client->queue_size()<4 && client->isConnected());     //如果没有收到响应消息则阻塞

    // 获取并显示对手是否存在
    msg=client->get_msg();          //获取房间对手是否有对手
    // 根据返回值设置对手头像：���对手显示头像，无对手显示空位图片
    msg=="1"?ui->label_opponent->setStyleSheet("QLabel{""border-image:url(:/new/prefix1/img/dog.png);""}"):
        ui->label_opponent->setStyleSheet("QLabel{""border-image:url(:/new/prefix1/img/none.png);""}");

    // 获取并显示对手准备状态
    msg=client->get_msg();          //获取对手是否已准备
    msg=="1"?ui->label_prepare_->setText("已准备") : ui->label_prepare_->setText("未准备");
    msg=="1"?ui->label_prepare_->setStyleSheet("QLabel{""color:green;""}") : ui->label_prepare_->setStyleSheet("QLabel{""color:red;""}");

    // 获取并显示对手IP
    msg=QString("IP:%1").arg(client->get_msg());        //获取对手IP
    ui->label_ip->setText(msg);

    // 获取并显示对手套接字FD
    msg=QString("FD:%1").arg(client->get_msg());        //获取对手套接字
    ui->label_fd->setText(msg);

    update();                       //更新视图
}

/**
 * @brief 定时器事件处理 - 核心消息处理循环
 * @param event 定时器事件（未使用）
 *
 * 每500ms触发一次，负责：
 * 1. 检测网络连接状态
 * 2. 游戏未开始时：获取准备信息，等待双方准备就绪
 * 3. 游戏进行中：处理落子、聊天、悔棋等消息
 * 4. 等待状态：处理悔棋响应
 *
 * 消息协议说明：
 * - "start": 双方准备就绪，游戏开始
 * - "c1": 己方为黑棋（先手）
 * - "c0": 己方为白棋（后手）
 * - "OMxy": 落子消息（x,y为坐标）
 * - "OR": 对手退出
 * - "OS": 对手认输
 * - "ONxxx": 聊天消息
 * - "OB": 悔棋请求
 * - "OB1"/"OB0": 悔棋响应（同意/拒绝）
 */
void internet_game::timerEvent(QTimerEvent *event)
{
    // 检测网络连接，断开则退出游戏
    if(!client->isConnected())              //网络连接中断退出
    {
        QMessageBox::information(this,"Warnning","网络连接中断",QMessageBox::Ok);
        client->clear();
        close();
        return ;
    }

    // ========== 游戏未开始状态 ==========
    if(!running)
    {
        ui->label_msg->hide();      // 隐藏回合提示

        // 检查是否收到游戏开始消息
        if(!client->queue_empty())
        {
            if(client->get_msg() == "start")      //双方已经准备就绪
            {
                // 初始化棋盘，切换到游戏界面
                initialization();
                ui->label_victory->hide();
                ui->stackedWidget->setCurrentIndex(1);  // 切换到游戏页面
                ui->button_black->show();               // 显示选择黑棋按钮
                ui->button_white->show();               // 显示选择白棋按钮
                ui->label_msg->show();
                ui->label_prepare->hide();
                ui->Label_your_color->hide();
                running=true;                      //正在运行
                //Sleep(300);
                return;                         //游戏已开始直接返回，不用再获取准备信息
            }
        }

        // 游戏未开始，持续获取对手准备信息
        get_prepare_information();          //获取对手准备信息
    }
    // ========== 游戏进行中 - 非等待状态 ==========
    else if(!wait)          //如果当前不处于等待状态
    {
        // ----- 等待双方选择先后手 -----
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
                    // 对手退出处理
                    on_Button_prepare_clicked();    //(游戏结束)调用准备按钮函数，相当于取消准备
                    ui->label_prepare->show();
                    ui->label_victory->setText("对手退出了游戏");
                    ui->label_victory->show();
                    ui->button_black->hide();
                    ui->button_white->hide();

                    // 创建定时器，5秒后自动隐藏提示
                    QTimer *timer = new QTimer(this);           //new一个定时器(5秒后发送timeout信号)
                    timer->start(5000);
                    connect(timer, &QTimer::timeout, this, [=](){
                        ui->label_victory->hide();
                        timer->stop();
                        delete timer;
                    });

                    // 返回准备界面
                    ui->stackedWidget->setCurrentIndex(0);
                    color = -1;
                    running = false;
                }

                // 处理先后手确认消息
                if(msg == "c1")         //如果是先手即黑方
                {
                    color = 1;          // 黑棋
                    turn = true;        //*轮到己方回合
                    ui->button_black->hide();
                    ui->button_white->hide();
                    ui->Label_your_color->show();
                    ui->Label_your_color->setStyleSheet("QLabel{border-image:url(:/new/prefix1/img/kuro.png)}");
                }
                else if(msg == "c0")         //如果是先手即黑方
                {
                    color = 0;          // 白棋
                    turn = false;       //*不是己方回合
                    ui->button_black->hide();
                    ui->button_white->hide();
                    ui->Label_your_color->show();
                    ui->Label_your_color->setStyleSheet("QLabel{border-image:url(:/new/prefix1/img/shiro.png)}");
                }
            }
        }
        // ----- 游戏正式开始后的消息处理 -----
        else                            //游戏开始后 处理服务器传来的消息
        {
            while(!client->queue_empty())
            {
                //处理对手落子后的信息(存储信息、回合交换、胜利判断)
                QString recv = client->get_msg();
                string msg = recv.toStdString();

                // 所有对战消息以'O'开头
                if(msg[0] == 'O')
                    switch(msg[1])
                    {
                    // ----- 对手落子消息 -----
                    case 'M':           //对手落子处理
                    {
                        // 解码坐标
                        //处理获得落子x,y的坐标
                        int x, y;
                        // 解码x坐标（a-e表示10-14）
                        if(msg[2] >= 'a' && msg[2] <= 'e')
                            x = msg[2] - 'a' + 10;
                        else
                            x = msg[2] - '0';
                        // 解码y坐标
                        if(msg[3] >= 'a' && msg[3] <= 'e')
                            y = msg[3] - 'a' + 10;
                        else
                            y = msg[3] - '0';

                        // 记录对手落子
                        chess_info[x][y].second = !color;           //存储对手的落子信息
                        back.push(QPair<int, int>(x, y));
                        update();           //更新棋盘
                        win(x, y);          //进行回合交换与胜利判断
                    }
                    break;

                    // ----- 对手退出房间 -----
                    case 'R':           //对手退出房间处理
                    {
                        on_Button_prepare_clicked();    //(游戏结束)调用准备按钮函数，相当于取消准备
                        ui->label_prepare->show();
                        ui->label_victory->setText("对手退出了游戏");
                        ui->label_victory->show();

                        // 5秒后自动隐藏提示
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

                    // ----- 对手认输 -----
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

                    // ----- 对手聊天消息 -----
                    case 'N':           //对手的聊天消息
                    {
                        // 提取消息内容（跳过前两个字符"ON"）
                        QString text = "[对手]:" + recv.mid(2);         //从下标为2的字符开始的字符串
                        ui->LE_recv->append(text);      // 追加到聊天记录框
                    }
                    break;

                    // ----- 对手悔棋请求 -----
                    case 'B':           //对手的悔棋请求处理
                    {
                        // 显示悔棋确认界面
                        ui->label_victory->setText("是否同意对手悔棋");
                        ui->label_victory->show();
                        ui->btn_back->setDisabled(true);    // 禁用己方悔棋按钮
                        ui->button_agree->show();           //同意按钮
                        ui->button_refuse->show();          //拒绝按钮
                        ban_mouse = true;                   // 禁用鼠标落子
                        return;
                    }
                    break;
                    default:
                        break;
                    }
            }
        }
    }
    // ========== 等待状态 - 等待悔棋响应 ==========
    else                    //等待状态下的处理 即等待悔棋请求响应
    {
        if(!client->queue_empty())          //如果收到对手的应答消息，同步本地客户端棋盘信息
        {
            string msg = client->get_msg().toStdString();

            // 处理悔棋响应消息 "OBx" (x为1同意，0拒绝)
            if(msg[0] == 'O' && msg.size() == 3)
                switch(msg[1])
                {
                case 'B':
                {
                    // 创建定时器用于显示响应结果
                    QTimer *timer = new QTimer(this);

                    if(msg[2] == '1')       // 对手同意悔棋
                    {
                        ui->label_anwser->setText("对手同意悔棋");
                        // 根据当前回合决定悔棋步数
                        if(turn)
                        {
                            go_back();go_back();     //己方回合后退2步（撤销对方和己方各一步）
                        }
                        else
                            go_back();              //对方回合后退1步（只撤销对方一步）
                    }
                    else                    // 对手拒绝悔棋
                    {
                        ui->label_anwser->setText("对手不同意悔棋");
                    }

                    // 显示响应结果，3秒后自动隐藏
                    ui->label_anwser->show();
                    connect(timer, &QTimer::timeout, this, [=](){
                        ui->label_anwser->hide();
                        timer->stop();
                        delete timer;
                    });
                    timer->start(3000);         //文字显示3秒(3秒后发送timeout信号)

                    // 结束等待状态
                    wait_over();
                }
                }
        }
    }
}


/**
 * @brief 鼠标点击事件处理
 * @param event 鼠标事件，包含点击位置信息
 *
 * 将鼠标点击位置传递给落子函数处理
 * 如果处于禁用状态（等待悔棋响应等），则忽略点击
 */
void internet_game::mousePressEvent(QMouseEvent *event)
{
    if(!ban_mouse)
        take_chess(event->x(), event->y());         //检测到鼠标点击事件并落子
}

/**
 * @brief 键盘按键事件处理
 * @param event 键盘事件
 *
 * 支持按回车键发送聊天消息
 */
void internet_game::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return)              //键盘回车键触发点击发送按钮
        on_Button_send_clicked();
}

/**
 * @brief 窗口关闭事件处理
 * @param event 关闭事件
 *
 * 关闭窗口时：
 * 1. 如果游戏正在进行，通知对手己方退出
 * 2. 发送gameOver信号，触发父窗口的清理操作
 */
void internet_game::closeEvent(QCloseEvent *event)
{
    qDebug() << "网络游戏页面关闭..." << Qt::endl;

    // 游戏进行中退出，需要通知对手
    if(running)                     //游戏运行中退出,通知对手已退出游戏
    {
        client->send_msg("OR");     //通过服务器将退出消息转发给对手
    }

    // 发送游戏结束信号，通知Menu窗口显示并清理资源
    emit gameOver();                 //发送游戏结束信号，触发网络页面析构
}

/**
 * @brief 退出房间按钮点击处理（准备界面）
 *
 * 关闭当前窗口，返回主菜单
 */
void internet_game::on_Button_exit_room_clicked()
{
    close();
}

/**
 * @brief 退出按钮点击处理（游戏界面）
 *
 * 关闭当前窗口，返回主菜单
 */
void internet_game::on_btn_exit_clicked()
{
    close();
}

/**
 * @brief 准备按钮点击处理
 *
 * 切换准备状态并通知服务器：
 * - 未准备 -> 已准备：显示"等待对手准备"
 * - 已准备 -> 未准备：显示"请准备"
 *
 * 当双方都准备就绪时，服务器会发送"start"消息
 */
//准备按钮点击
void internet_game::on_Button_prepare_clicked()
{
    int ret;
    //Sleep(500);

    // 向服务器发送准备状态变更请求
    ret = client->send_msg("prepare");      //发送准备请求
    if(ret==SOCKET_ERROR)
        return;

    // 根据当前状态更新UI显示
    //此时的prepare是点击前的状态
    else if(prepare)        // 当前已准备，点击后变为未准备
    {
        ui->label_prepare->setText("请准备...");
        ui->label_prepare->setStyleSheet("QLabel{color:rgba(255,0,0,0.6);}");   // 红色提示
    }
    else                    // 当前未准备，点击后变为已准备
    {
        ui->label_prepare->setText("等待对手准备...");
        ui->label_prepare->setStyleSheet("QLabel{color:rgba(0,255,0,0.6);}");   // 绿色提示
    }

    // 切换准备状态
    prepare=!prepare;                   //更新准备状态

    // 更新按钮文字
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

/**
 * @brief 选择黑棋（先手）按钮点击处理
 *
 * 向服务器发送选择先手的请求
 * 服务器会通知对方己方的选择
 */
//先手按钮点击
void internet_game::on_button_black_clicked()
{
    // 已经选择过颜色则忽略
    if(color != -1)
        return;
    client->send_msg("color1");     // 发送选择黑棋（1）的请求
}

/**
 * @brief 选择白棋（后手）按钮点击处理
 *
 * 向服务器发送选择后手的请求
 */
//后手按钮点击
void internet_game::on_button_white_clicked()
{
    // 已经选择过颜色则忽略
    if(color != -1)
        return;
    client->send_msg("color0");     // 发送选择白棋（0）的请求
}

/**
 * @brief 认输按钮点击处理
 *
 * 认输流程：
 * 1. 向服务器发送认输消息"OS"
 * 2. 服务器转发给对手
 * 3. 本地显示认输结果
 * 4. 返回准备界面
 */
//认输按钮点击
void internet_game::on_btn_surrender_clicked()
{
    // 游戏未开始或未选择颜色时不能认输
    if(!running || color == -1)
        return;

    client->send_msg("OS");             //向服务器发送认输消息,通知对手已认输

    // 本地处理：取消准备状态，显示结果
    on_Button_prepare_clicked();
    ui->label_prepare->show();
    ui->label_victory->setText("你已认输");
    ui->label_victory->show();
    ui->stackedWidget->setCurrentIndex(0);  // 返回准备界面
    color = -1;
    running = false;
}

/**
 * @brief 发送聊天消息按钮点击处理
 *
 * 发送流程：
 * 1. 检查输入框是否为空
 * 2. 构造消息格式"ON+内容"
 * 3. 在本地聊天框显示
 * 4. 发送给服务器（服务器转发给对手）
 */
//发送按钮点击
void internet_game::on_Button_send_clicked()
{
    // 空消息不发送
    if(ui->LE_send->text() == "")
        return;

    // 构造消息：ON + 消息内容
    QString msg = "ON" + ui->LE_send->text();

    // 在本地聊天框显示己方消息
    ui->LE_recv->append("[你]:" + ui->LE_send->text());

    // 清空输入框
    ui->LE_send->clear();

    // 发送给服务器
    client->send_msg(msg);      //将消息发送给服务器
}

/**
 * @brief 悔棋按钮点击处理
 *
 * 悔棋请求流程：
 * 1. 检查游戏状态（必须在游戏进行中且���落子记录）
 * 2. 发送悔棋请求"OB"给服务器
 * 3. 进入等待状态，等待对手响应
 * 4. 禁用鼠标和部分按钮
 */
//悔棋按钮点击
void internet_game::on_btn_back_clicked()
{
    // 游戏未开始、未选择颜色或没有落子记录时不能悔棋
    if(!running || color == -1 || back.empty())
        return;

    client->send_msg("OB");         //向服务器发送悔棋请求

    // 显示等待提示
    ui->label_wait_answer->setText("请等待对手做出回应");
    ui->label_wait_answer->show();

    // 进入等待状态
    ban_mouse = true;              //禁用鼠标
    wait = true;                   //等待状态
}

/**
 * @brief 同意对手悔棋按钮点击处理
 *
 * 同意悔棋流程：
 * 1. 发送同意消息"OB1"给服务器
 * 2. 根据当前回合执行悔棋操作
 *    - 对方回合：退回2步（撤销双方各一步）
 *    - 己方回合：退回1步（只撤销对方一步）
 * 3. 结束等待状态，恢复正常游戏
 */
//同意悔棋按钮
void internet_game::on_button_agree_clicked()
{
    client->send_msg("OB1");         //向服务器发送悔棋同意信息

    // 根据当前回合决定悔棋步数
    if(!turn)               //如果不是己方回合 则向前退回两步
    {
        go_back();
        go_back();
    }
    else
        go_back();

    // 结束等待状态
    wait_over();
}

/**
 * @brief 拒绝对手悔棋按钮点击处理
 *
 * 拒绝悔棋流程：
 * 1. 发送拒绝消息"OB0"给服务器
 * 2. 结束等待状态，恢复正常游戏
 *
 * 注意：拒绝悔棋不需要修改棋盘状态
 */
//拒绝悔棋按钮
void internet_game::on_button_refuse_clicked()
{
    client->send_msg("OB0");        //向服务器发送悔棋拒绝信息
    wait_over();
}
