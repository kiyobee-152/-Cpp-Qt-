#include "client_net.h"

//初始化客户端套接字与目标ip地址与端口
client_net::client_net()
{
    WSAStartup(MAKEWORD(2, 2), &wsadata);
    client_fd = socket(PF_INET, SOCK_STREAM, 0);            //[BUG]建立tcp连接(重要*)
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr("192.168.152.129");  //你的服务器的公网IP
    client_addr.sin_port = htons(4396);                         //目的端口号
    connected = false;
    connect_thread_running = false;
    received = false;
}

client_net::~client_net()
{
    qDebug() << "client_net 析构..." << Qt::endl;
    closesocket(client_fd);
    WSACleanup();
}

//获取客户端套接字
SOCKET client_net::get_socket_fd()
{
    return client_fd;
}

bool client_net::isConnected()
{
    return connected;
}

//连接服务器
bool client_net::connect()
{
    //SOCKET fd = get_socket_fd();
    if(isConnected())
        return true;
    connect_thread_running = true;      //正在连接
    client_fd = socket(PF_INET, SOCK_STREAM, 0);        //[BUG] 重新建立套接字 否则重新连接不上
    for(int i = 0; i < 10 && connect_thread_running; i++)
    {
        //尝试连接
        int ret = ::connect(client_fd, (struct sockaddr*)&client_addr, sizeof (client_addr));
        //连接成功并接收服务器数据
        if(ret == 0)
        {
            connected = true;
            received = true;
            connect_thread_running = false;
            qDebug() << "连接服务器成功" <<Qt::endl;
            _beginthreadex(NULL, 0, recv_msg, this, 0, NULL);
            return true;
        }
        qDebug() << "请求连接中:" << i <<Qt::endl;
    }
    connect_thread_running = false;
    return false;
}

//断开连接
void client_net::disconnect()
{
    if(!connected)
        return;
    connected = false;
    received = false;
    qDebug() << "客户端断开连接" << Qt::endl;
    closesocket(client_fd);         //关闭套接字
}

void client_net::set_addr(QString str)
{
    if(connected)
        return;
    if( str.isNull())
        return;
    client_addr.sin_addr.s_addr = inet_addr(str.toUtf8().data());
}

void client_net::set_port(QString str)
{
    if(connected)
        return;
    client_addr.sin_port = htons(str.toInt());
}

int client_net::send_msg(QString msg)
{
    if(connected)
    {
        //[BUG] strlen(msg.toUtf8().data())不能写成sizeof()
        return send(client_fd, msg.toUtf8().data(), strlen(msg.toUtf8().data()), 0);
    }
    else
        return SOCKET_ERROR;

}

QString client_net::get_msg()
{
    if(!connected||msg_queue.size()==0)
        return "";
    QString msg = msg_queue.front();        //获取队头数据
    qDebug() << "get msg: " << msg <<Qt::endl;
    msg_queue.pop_front();
    return msg;
}

void client_net::push_msg(QString msg)
{
    msg_queue.push_back(msg);
    qDebug() << "push msg: " << msg << Qt::endl;
}

void client_net::msg_handle(QString msg)
{
    for(int i = 0; i < msg.size(); i++)
    {
        //如果遇到/则开始截取后面直到下一个/前的消息字符串
        if(msg[i] == '/' && (i + 1 < msg.size()))
        {
            /*
            switch(msg[i+1].unicode())
            {
                case 'N':i=msg_end(i+2,res)-1;break;
                case 'S':i=msg_end(i+2,res)-1;break;
                case 'I':i=msg_end(i+2,res)-1;break;
                case 'F':i=msg_end(i+2,res)-1;break;
                case 'Z':i=msg_end(i+2,res)-1;break;
                default:i=msg_end(i+2,res)-1;break;
            }
            */
            //消息首字符为N S I F Z
            if((msg[i + 1].unicode() == 'N') || (msg[i + 1].unicode() == 'S') ||
                (msg[i + 1].unicode() == 'I') || (msg[i + 1].unicode() == 'F') || (msg[i + 1].unicode() == 'Z'))
                i = msg_end(i + 2, msg) - 1;        //i+2是消息字符串首地址; -1是退回到/前一个字符以便再次检验到/截取下一个消息字符串
        }
        else
        {
            //[BUG]游戏运行过程中接收到的服务器不作处理原地转发的对手的消息直接存入队列(如聊天消息、落子信息等)
            push_msg(msg);
            return;
        }
    }
}

//处理字符串(截取/之间的内容)
int client_net::msg_end(int index, QString str)
{
    if(index == str.size())
        return index;
    QString ret;
    for(; index < str.size(); index++)
    {
        //遇到/截取完毕 放入消息队列中
        if(str[index] == '/')
        {
            push_msg(ret);
            return index;
        }
        else
        {
            ret += str[index];
        }
    }
    push_msg(ret);
    return index;
}

void client_net::clear()
{
    msg_queue.clear();
}

bool client_net::queue_empty()
{
    return msg_queue.empty();
}

int client_net::queue_size()
{
    return msg_queue.size();
}

//接收数据
unsigned WINAPI recv_msg(void *arg)
{
    client_net *net = (client_net*)arg;
    SOCKET fd = net->get_socket_fd();
    char msg[1024];
    while(1)
    {
        memset(msg, 0, sizeof(msg));     //[BUG]*在循环体内必须初始化，否则套接字字符后有脏数据
        int ret = recv(fd, msg, sizeof(msg), 0);
        //客户端主动断开连接时 由于closesocket(fd) 所以recv必然调用失败
        if(ret < 0)
        {
            qDebug() << "与服务器断开连接" <<Qt::endl;
            net->disconnect();
            return NULL;
        }
        //接收到服务器端发送的EOF 说明服务器已被关闭
        else if(ret == 0)
        {
            qDebug() << "服务器已关闭..." << Qt::endl;
            net->disconnect();
            return NULL;
        }
        qDebug() << "recv msg:" << msg << Qt::endl;
        net->msg_handle(msg);           //处理数据(存入消息队列中)
    }
    return NULL;
}
