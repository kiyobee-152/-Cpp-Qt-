#ifndef INTERNET_GAME_H
#define INTERNET_GAME_H

#include <QWidget>
#include <QStack>
#include <QLabel>
#include <QCloseEvent>
#include <QPainter>
#include <QPaintEvent>
#include "QMouseEvent"
#include <QDebug>
#include <QtWidgets>
#include "client_net.h"
#include <QTimer>
#include <stdlib.h>
#include <stdio.h>
#include <QMessageBox>

namespace Ui {
class internet_game;
}

class internet_game : public QWidget
{
    Q_OBJECT

public:
    explicit internet_game(QWidget *parent = nullptr);
    internet_game(int x,int y,client_net *client_,QString room_name,QWidget *parent = nullptr);
    ~internet_game();

    client_net *client;         //网络通信对象

private:
    Ui::internet_game *ui;

    int square;                //每个格子的平均边长
    QPixmap white_chess;       //白棋图片
    QPixmap black_chess;       //黑棋图片
    QPixmap board_bg;          //棋盘背景

    QVector<QVector<QPair<QRect, int>>> chess_info;         //棋盘信息 记录每个点的落子颜色与落子范围等
    QStack<QPair<int, int>> back;                          //所有落子信息 悔棋用

    bool wait;//用于游戏运行中，一方发出悔棋、新游戏的请求后发出方持续的状态，这个状态下发出方将只等待处理对方的回应信息
    bool turn;//用于游戏运行中，你的回合，为你的回合时才能下棋，但此时，依然可以点击悔棋、新游戏等按钮
    int timerId1;
    int color;//颜色，先后手，0为白棋，1为黑棋，其他值为游戏尚未开始
    bool running;//游戏运行与否，为false则代表游戏处于等待状态，需要两个玩家，并且都准备
    bool prepare;//存放准备按钮的值，0为未准备，1为准备。
    bool ban_mouse;     //是否禁用鼠标

public:
    void initialization();          //初始化棋盘
    void take_chess(int ,int );     //落子函数
    void win(int x,int y);//判断胜利条件
    void go_back();                 //悔棋操作
    void get_prepare_information();
    void wait_over();       //等待状态结束

protected:
      void closeEvent(QCloseEvent *event);
      void paintEvent(QPaintEvent *);
      void mousePressEvent(QMouseEvent *event);
      void timerEvent(QTimerEvent* event);
      void keyPressEvent(QKeyEvent * event);

signals:
    void gameOver();        //游戏结束信号（关闭事件触发时发出）

private slots:
    void on_Button_exit_room_clicked();
    void on_btn_exit_clicked();
    void on_Button_prepare_clicked();
    void on_button_black_clicked();
    void on_button_white_clicked();
    void on_btn_surrender_clicked();
    void on_Button_send_clicked();
    void on_btn_back_clicked();
    void on_button_agree_clicked();
    void on_button_refuse_clicked();
};

#endif // INTERNET_GAME_H
