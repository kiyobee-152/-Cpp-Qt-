#ifndef GAMEWIN_H
#define GAMEWIN_H

#include <QWidget>
#include <QStack>
#include <QLabel>
#include <QCloseEvent>
#include <QPainter>
#include <QPaintEvent>
#include "QMouseEvent"
#include <QDebug>

namespace Ui {
class GameWin;
}

class GameWin : public QWidget
{
    Q_OBJECT

public:
    explicit GameWin(QWidget *parent = nullptr);
    ~GameWin();

public:
    void initialization();          //重新游戏 初始化棋盘
    void press_event(int, int);              //鼠标点击事件处理
    void win(int, int);             //判断是否胜利

public:
    void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    Ui::GameWin *ui;

    int square;                //每个格子的平均边长
    QPixmap white_chess;       //白棋图片
    QPixmap black_chess;       //黑棋图片
    QPixmap board_bg;          //棋盘背景

    bool black = true;          //是否黑子回合 否则白子回合
    bool running;               //游戏是否运行
    QVector<QVector<QPair<QRect, int>>> chess_info;         //棋盘信息 记录每个点的落子颜色与落子范围等
    QStack<QPair<int, int>> back;                          //所有落子信息 悔棋用

signals:
    void gameOver();        //游戏结束信号（关闭事件触发时发出）


private slots:
    void on_exit_btn_clicked();
    void on_new_btn_clicked();
    void on_back_btn_clicked();
};

#endif // GAMEWIN_H
