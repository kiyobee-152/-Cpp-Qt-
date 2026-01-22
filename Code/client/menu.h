#ifndef MENU_H
#define MENU_H

#include <QWidget>
#include <QtWidgets>
#include <stdlib.h>
#include <stdio.h>
#include "client_net.h"
#include "internet_game.h"
#include "gamewin.h"
#include <QStandardItemModel>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class Menu; }
QT_END_NAMESPACE

class Menu : public QWidget
{
    Q_OBJECT

public:
    Menu(QWidget *parent = nullptr);
    ~Menu();

    client_net *client;
    int timerId;                //定时器id
    void timerEvent(QTimerEvent *event);        //定时器
    QStandardItemModel *tableModel;
    QStringList columnTitle;

    void join_game();

private slots:
    void on_local_game_btn_clicked();

    void on_about_btn_clicked();

    void on_exit_btn_clicked();

    void on_net_game_btn_clicked();

    void on_exit_net_btn_clicked();

    void on_reconnect_btn_clicked();

    void on_create_btn_clicked();

    void on_refresh_btn_clicked();

private:
    Ui::Menu *ui;
};
#endif // MENU_H
