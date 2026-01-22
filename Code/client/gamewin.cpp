#include "gamewin.h"
#include "ui_gamewin.h"


//棋盘横竖各15条线
#define chessboard_size 15

GameWin::GameWin(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameWin)
{
    ui->setupUi(this);

    setWindowIcon(QPixmap(":new/prefix1/img/Title1.png"));
    setWindowTitle("五子棋本地对战");
    white_chess.load(":/new/prefix1/img/shiro.png");
    black_chess.load(":/new/prefix1/img/kuro.png");
    board_bg.load(":/new/prefix1/img/btnbg.jpg");

    square = 800 / (chessboard_size + 1);           //格子边长赋值
    //保存每个点的信息  [BUG]下标必须从0开始
    for(int i = 0; i < chessboard_size; i++)
    {
        chess_info.push_back(QVector<QPair<QRect, int>>());
        for(int j = 0; j < chessboard_size; j++)
        {
            //设置每个点的点击范围
            chess_info[i].push_back(QPair<QRect, int>(QRect((i+1)*square - square*1.25/3, (j+1)*square - square*1.25/3, square/3*2.5, square/3*2.5), -1));
        }
    }

    initialization();
}

GameWin::~GameWin()
{
    delete ui;
}

void GameWin::closeEvent(QCloseEvent *)
{
    qDebug() << "关闭事件触发";
    emit GameWin::gameOver();       //发出游戏结束信号
}

//画棋盘
void GameWin::paintEvent(QPaintEvent *)
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

    //黑子回合显示黑子图片
    if(black)
    {
        ui->chess_label->setStyleSheet("border-image:url(:new/prefix1/img/kuro.png);");
    }
    else
    {
        ui->chess_label->setStyleSheet("border-image:url(:new/prefix1/img/shiro.png);");
    }

    //画棋子
    for(auto &x : chess_info)
    {
        for(auto &y : x)
        {
            switch (y.second)
            {
                case 1:
                    painter.drawPixmap(y.first, black_chess);
                    break;
                case 0:
                    painter.drawPixmap(y.first, white_chess);
                    break;
                case -1:
                    break;
                default:break;
            }
        }
    }
    if(!back.empty())
    {
        pen.setWidth(8);
        pen.setColor(QColor(Qt::red));
        painter.setPen(pen);
        painter.drawPoint((back.top().first + 1) * 50, (back.top().second + 1) * 50);
    }
}

void GameWin::mousePressEvent(QMouseEvent *event)
{
    press_event(event->x(), event->y());
}


void GameWin::initialization()
{
    black = true;
    running = true;
    back.resize(0);
    ui->back_btn->setDisabled(false);
    ui->chessboard->setText("");
    ui->chessboard->setStyleSheet("color:red");
    for(auto &x : chess_info)
    {
        for(auto &y : x)
        {
            y.second = -1;
        }
    }
    update();       //更新窗口
}


void GameWin::press_event(int x, int y)
{
    if(!running)
        return;
    qDebug() << "点击位置:" << x << " " << y;
    QRect r(x, y, 5, 5);        //点击的触摸区域
    for(int i = 0; i < chess_info.size(); i++)
    {
        for(int j = 0; j < chess_info[i].size(); j++)
        {
            //如果点击位置相交且该点没有落子
            if(chess_info[i][j].first.intersects(r) && chess_info[i][j].second == -1)
            {
                chess_info[i][j].second = black;        //记录该点落子颜色
                black = !black;
                back.push(QPair<int, int>(i, j));
                qDebug() << i << " " << j;
                win(i, j);
            }
        }
    }
    update();
}

void GameWin::win(int x, int y)
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
            if(a >= 0 && b >= 0 && a < chess_info.size() && b < chess_info.size() && chess_info[a][b].second == (!black))
            {
                sum++;
            }
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
            if(a >= 0 && b >= 0 && a < chess_info.size() && b < chess_info.size() && chess_info[a][b].second == (!black))
            {
                sum++;
            }
            else
                break;
        }

        if(sum >= 4)
        {
            if(black)
                ui->chessboard->setText("白方胜利");
            else
                ui->chessboard->setText("黑方胜利");
            running = false;
            ui->back_btn->setDisabled(true);
        }
    }
}

//退出游戏按钮事件
void GameWin::on_exit_btn_clicked()
{
    close();
}

void GameWin::on_new_btn_clicked()
{
    initialization();
}

//悔棋按钮事件
void GameWin::on_back_btn_clicked()
{
    if(back.empty())
        return;
    QPair<int, int> p = back.top();
    chess_info[p.first][p.second].second = -1;
    back.pop();
    black = !black;
    update();
}
