/*
 * 客户端网络连接
 * 处理与目标服务器的连接以及消息字符串收发与处理
*/

#ifndef CLIENT_NET_H
#define CLIENT_NET_H

#include <QQueue>
#include <iostream>
#include <stdio.h>
#include <winsock2.h>
#include <QString>
#include <process.h>
#include <QDebug>

using namespace std;

class client_net
{
public:
    client_net();
    ~client_net();
    SOCKET get_socket_fd();
    bool isConnected();             //返回connected
    void set_addr(QString);         //设置目标服务器地址
    void set_port(QString);         //设置目标服务器端口
    bool connect();                 //连接服务器
    void disconnect();              //断开连接
    int send_msg(QString);         //向服务器发送数据
    void push_msg(QString msg);                //向消息队列中加入数据
    QString get_msg();              //向消息队列中取数据
    void msg_handle(QString msg);   //处理服务器发来的数据
    int msg_end(int index, QString str);           //截取消息字符串
    void clear();                   //清理消息队列
    bool queue_empty();
    int queue_size();
    bool connect_thread_running;    //是否正字连接

private:
    WSADATA wsadata;
    SOCKADDR_IN client_addr;        //目标服务器地址
    SOCKET client_fd;               //客户端套接字
    bool connected;                 //是否已经连接(只读)
    bool received;                  //是否可接收数据(只读)

    QQueue<QString> msg_queue;      //消息队列
};

//进行C++thread多线程编程时线程调用的程序必须加WINAPI宏形式声明
unsigned WINAPI recv_msg(void *arg);            //接收服务器发来的数据

#endif // CLIENT_NET_H
