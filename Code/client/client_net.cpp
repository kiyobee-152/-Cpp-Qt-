/**
 * @file client_net.cpp
 * @brief 客户端网络通信模块实现
 *
 * 本文件实现了客户端与服务器之间的网络通信功能，包括：
 * - TCP连接的建立与断开
 * - 消息的发送与接收
 * - 消息队列的管理与消息协议的解析
 *
 * 使用Windows Socket API (WinSock2) 实现网络通信
 * 采用多线程方式异步接收服务器消息
 */

#include "client_net.h"

/**
 * @brief 构造函数 - 初始化客户端套接字与目标服务器地址
 *
 * 执行以下初始化操作：
 * 1. 初始化WinSock库（版本2.2）
 * 2. 创建TCP套接字
 * 3. 配置目标服务器的IP地址和端口号
 * 4. 初始化连接状态标志位
 */
//初始化客户端套接字与目标ip地址与端口
client_net::client_net()
{
    // 初始化Windows Socket库，请求版本2.2
    // MAKEWORD(2, 2)表示主版本号2，副版本号2
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    // 创建TCP套接字
    // PF_INET: IPv4协议族
    // SOCK_STREAM: 流式套接字（TCP）
    // 0: 自动选择协议（TCP）
    client_fd = socket(PF_INET, SOCK_STREAM, 0);            //[BUG]建立tcp连接(重要*)

    // 设置目标服务器地址结构体
    client_addr.sin_family = AF_INET;                              // 使用IPv4地址族
    client_addr.sin_addr.s_addr = inet_addr("192.168.152.129");  //你的服务器的公网IP
    client_addr.sin_port = htons(4396);                         //目的端口号 htons将主机字节序转换为网络字节序

    // 初始化状态标志
    connected = false;              // 当前未连接服务器
    connect_thread_running = false; // 连接线程未运行
    received = false;               // 未准备好接收数据
}

/**
 * @brief 析构函数 - 清理网络资源
 *
 * 关闭套接字并清理WinSock库占用的资源
 * 在对象销毁时自动调用，确保网络资源被正确释放
 */
client_net::~client_net()
{
    qDebug() << "client_net 析构..." << Qt::endl;
    closesocket(client_fd);     // 关闭套接字，释放系统资源
    WSACleanup();               // 清理WinSock库，释放DLL资源
}

/**
 * @brief 获取客户端套接字描述符
 * @return SOCKET 返回客户端套接字文件描述符
 *
 * 用于在其他函数（如接收线程）中访问套接字
 */
//获取客户端套接字
SOCKET client_net::get_socket_fd()
{
    return client_fd;
}

/**
 * @brief 检查客户端是否已连接到服务器
 * @return bool true表示已连接，false表示未连接
 *
 * 外部调用此方法判断当前网络连接状态
 */
bool client_net::isConnected()
{
    return connected;
}

/**
 * @brief 连接到服务器
 * @return bool 连接成功返回true，失败返回false
 *
 * 连接流程：
 * 1. 检查是否已连接，避免重复连接
 * 2. 重新创建套接字（解决重连问题）
 * 3. 循环尝试连接（最多10次）
 * 4. 连接成功后启动接收线程
 *
 * 注意：使用::connect()调用全局connect函数，避免与成员函数名冲突
 */
//连接服务器
bool client_net::connect()
{
    //SOCKET fd = get_socket_fd();

    // 如果已经连接，直接返回成功
    if(isConnected())
        return true;

    connect_thread_running = true;      //正在连接 设置连接中状态标志

    // 重新创建套接字
    // 原因：之前的套接字可能已被关闭或处于异常状态，重新创建确保连接可用
    client_fd = socket(PF_INET, SOCK_STREAM, 0);        //[BUG] 重新建立套接字 否则重新连接不上

    // 循环尝试连接，最多尝试10次
    // connect_thread_running用于外部控制，可随时终止连接尝试
    for(int i = 0; i < 10 && connect_thread_running; i++)
    {
        //尝试连接
        // ::connect是全局函数，用于建立TCP连接
        // 参数：套接字、服务器地址结构体指针、地址结构体大小
        int ret = ::connect(client_fd, (struct sockaddr*)&client_addr, sizeof (client_addr));

        //连接成功并接收服务器数据
        // connect返回0表示连接成功
        if(ret == 0)
        {
            connected = true;               // 标记为已连接状态
            received = true;                // 标记为可接收数据状态
            connect_thread_running = false; // 连接过程结束
            qDebug() << "连接服务器成功" <<Qt::endl;

            // 启动数据接收线程
            // _beginthreadex: Windows多线程函数，创建新线程执行recv_msg函数
            // 参数：安全属性、栈大小、线程函数、参数、创建标志、线程ID指针
            // this指针作为参数传入，使接收线程可以访问当前对象
            _beginthreadex(NULL, 0, recv_msg, this, 0, NULL);
            return true;
        }
        qDebug() << "请求连接中:" << i <<Qt::endl;
    }

    // 10次尝试均失败，连接失败
    connect_thread_running = false;
    return false;
}

/**
 * @brief 断开与服务器的连接
 *
 * 断开流程：
 * 1. ��查是否处于连接状态
 * 2. 重置状态标志
 * 3. 关闭套接字
 *
 * 关闭套接字后，接收线程中的recv()会返回错误，从而自动退出
 */
//断开连接
void client_net::disconnect()
{
    // 如果未连接，无需断开
    if(!connected)
        return;

    // 重置连接状态标志
    connected = false;      // 标记为未连接
    received = false;       // 标记为不可接收数据
    qDebug() << "客户端断开连接" << Qt::endl;
    closesocket(client_fd);         //关闭套接字 这会导致recv_msg线程中的recv()调用失败并退出
}

/**
 * @brief 设置目标服务器IP地址
 * @param str 服务器IP地址字符串（如"192.168.1.1"）
 *
 * 注意：只有在未连接状态下才能修改服务器地址
 * inet_addr()将点分十进制IP字符串转换为网络字节序的32位整数
 */
void client_net::set_addr(QString str)
{
    // 已连接状态下不允许修改地址
    if(connected)
        return;
    // 空字符串检查
    if( str.isNull())
        return;
    // 将QString转换为C风格字符串，再转换为网络地址
    client_addr.sin_addr.s_addr = inet_addr(str.toUtf8().data());
}

/**
 * @brief 设置目标服务器端口号
 * @param str 端口号字符串（如"4396"）
 *
 * 注意：只有在未连接状态下才能修改端口号
 * htons()将主机字节序(小端)转换为网络字节序(大端)
 */
void client_net::set_port(QString str)
{
    // 已连接状态下不允许修改端口
    if(connected)
        return;
    // 将字符串转换为整数，再转换为网络字节序
    client_addr.sin_port = htons(str.toInt());
}

/**
 * @brief 向服务器发送消息
 * @param msg 要发送的消息内容（QString类型）
 * @return int 发送成功返回发送的字节数，失败返回SOCKET_ERROR
 *
 * 发送流程：
 * 1. 检查连接状态
 * 2. 将QString转换为UTF-8编码的字节数组
 * 3. 调用send()函数发送数据
 *
 * 注意：必须使用strlen获取实际字符串长度，不能使用sizeof
 */
int client_net::send_msg(QString msg)
{
    if(connected)
    {
        // 将QString转换为UTF-8字节数组并发送
        // send()参数：套接字、数据缓冲区、数据长度、标志位
        //[BUG] strlen(msg.toUtf8().data())不能写成sizeof()
        // sizeof返回指针大小(8字节)，而非字符串实际长度
        return send(client_fd, msg.toUtf8().data(), strlen(msg.toUtf8().data()), 0);
    }
    else
        return SOCKET_ERROR;    // 未连接时返回错误

}

/**
 * @brief 从消息队列中获取一条消息
 * @return QString 返回队列头部的消息，队列为空或未连接时返回空字符串
 *
 * 使用FIFO（先进先出）方式从队列中取出消息
 * 取出后该消息从队列中移除
 */
QString client_net::get_msg()
{
    // 未连接或队列为空时返回空字符串
    if(!connected||msg_queue.size()==0)
        return "";
    QString msg = msg_queue.front();        //获取队头数据
    qDebug() << "get msg: " << msg <<Qt::endl;
    msg_queue.pop_front();                  // 移除队头元素
    return msg;
}

/**
 * @brief 向消息队列中添加一条消息
 * @param msg 要添加的消息内容
 *
 * 将解析后的消息添加到队列尾部，供上层应用读取处理
 */
void client_net::push_msg(QString msg)
{
    msg_queue.push_back(msg);               // 添加到队列尾部
    qDebug() << "push msg: " << msg << Qt::endl;
}

/**
 * @brief 处理从服务器接收到的原始消息
 * @param msg 从服务器接收到的原始消息字符串
 *
 * 消息协议格式说明：
 * - 服务器发送的消息以'/'作为分隔符
 * - 消息类型标识符紧跟在'/'后面：
 *   - N: 房间信息相关
 *   - S: 状态信息相关
 *   - I: 信息相关
 *   - F: 失败信息
 *   - Z: 其他信息
 * - 格式示例：/N房间数据/S状态数据/
 *
 * 处理逻辑：
 * 1. 遍历消息字符串，查找'/'分隔符
 * 2. 根据消息类型标识符截取对应消息内容
 * 3. 非协议格式的消息（如聊天消息）直接存入队列
 */
void client_net::msg_handle(QString msg)
{
    for(int i = 0; i < msg.size(); i++)
    {
        //如果遇到/则开始截取后面直到下一个/前的消息字符串
        // 检查是否为协议消息的起始标识
        if(msg[i] == '/' && (i + 1 < msg.size()))
        {
            /*
            // 原始switch实现（已注释）
            switch(msg[i+1].unicode())
            {
                case 'N':i=msg_end(i+2,res)-1;break;  // N类型消息
                case 'S':i=msg_end(i+2,res)-1;break;  // S类型消息
                case 'I':i=msg_end(i+2,res)-1;break;  // I类型消息
                case 'F':i=msg_end(i+2,res)-1;break;  // F类型消息
                case 'Z':i=msg_end(i+2,res)-1;break;  // Z类型消息
                default:i=msg_end(i+2,res)-1;break;
            }
            */
            //消息首字符为N S I F Z
            // 判断是否为有效的协议消息类型
            if((msg[i + 1].unicode() == 'N') || (msg[i + 1].unicode() == 'S') ||
                (msg[i + 1].unicode() == 'I') || (msg[i + 1].unicode() == 'F') || (msg[i + 1].unicode() == 'Z'))
                // 调用msg_end截取消息内容
                // i+2: 跳过'/'和类型标识符，指向消息内容起始位置
                // -1: 因为for循环会i++，所以需要退回一位，确保不会跳过下一个'/'
                i = msg_end(i + 2, msg) - 1;        //i+2是消息字符串首地址; -1是退回到/前一个字符以便再次检验到/截取下一个消息字符串
        }
        else
        {
            // 非协议格式消息（如对手发送的聊天消息、落子坐标等）
            // 这类消息由服务器原样转发，直接存入队列等待处理
            //[BUG]游戏运行过程中接收到的服务器不作处理原地转发的对手的消息直接存入队列(如聊天消息、落子信息等)
            push_msg(msg);
            return;     // 直接返回，不继续解析
        }
    }
}

/**
 * @brief 截取消息字符串（从指定位置到下一个'/'之前）
 * @param index 截取起始位置索引
 * @param str 原始消息字符串
 * @return int 返回截取结束位置的索引（'/'的位置或字符串末尾）
 *
 * 工作原理：
 * 1. 从index位置开始遍历字符串
 * 2. 逐字符拼接直到遇到'/'或字符串结束
 * 3. 将截取的子字符串存入消息队列
 */
//处理字符串(截取/之间的内容)
int client_net::msg_end(int index, QString str)
{
    // 边界检查：如果起始位置已到字符串末尾，直接返回
    if(index == str.size())
        return index;

    QString ret;    // 用于存储截取的消息内容

    for(; index < str.size(); index++)
    {
        //遇到/截取完毕 放入消息队列中
        // 遇到'/'表示当前消息结束
        if(str[index] == '/')
        {
            push_msg(ret);      // 将截取的消息存入队列
            return index;       // 返回'/'的位置
        }
        else
        {
            ret += str[index];  // 拼接字符到结果字符串
        }
    }

    // 字符串结束但未遇到'/'，将剩余内容存入队列
    push_msg(ret);
    return index;   // 返回字符串末尾位置
}

/**
 * @brief 清空消息队列
 *
 * 在特定场景下（如进入新游戏房间）需要清空之前遗留的消息
 */
void client_net::clear()
{
    msg_queue.clear();
}

/**
 * @brief 检查消息队列是否为空
 * @return bool 队列为空返回true，否则返回false
 */
bool client_net::queue_empty()
{
    return msg_queue.empty();
}

/**
 * @brief 获取消息队列中的消息数量
 * @return int 返回队列中的消息数量
 */
int client_net::queue_size()
{
    return msg_queue.size();
}

/**
 * @brief 数据接收线程函数（独立线程运行）
 * @param arg 线程参数，传入client_net对象指针
 * @return unsigned 线程返回值，正常退出返回NULL
 *
 * 线程工作流程：
 * 1. 无限循环等待接收服务器数据
 * 2. 接收到数据后调用msg_handle()解析并存入队列
 * 3. 连接断开或服务器关闭时退出线程
 *
 * 线程退出条件：
 * - recv()返回负值：客户端主动断开（套接字被关闭）
 * - recv()返回0：服务器发送EOF（服务器关闭）
 *
 * 注意：此函数声明为WINAPI调用约定，用于Windows多线程
 */
//接收数据
unsigned WINAPI recv_msg(void *arg)
{
    // 将void*参数转换为client_net指针，获取网络对象
    client_net *net = (client_net*)arg;
    SOCKET fd = net->get_socket_fd();   // 获取套接字描述符
    char msg[1024];                     // 接收缓冲区，最大接收1024字节

    // 无限循环接收数据
    while(1)
    {
        // 每次接收前清空缓冲区，防止脏数据残留
        memset(msg, 0, sizeof(msg));     //[BUG]*在循环体内必须初始化，否则套接字字符后有脏数据

        // 阻塞等待接收服务器数据
        // recv()参数：套接字、缓冲区、缓冲区大小、标志位
        // 返回值：接收到的字节数，0表示连接关闭，负值表示错误
        int ret = recv(fd, msg, sizeof(msg), 0);

        //客户端主动断开连接时 由于closesocket(fd) 所以recv必然调用失败
        // ret < 0: 接收失败，通常是因为套接字被关闭（客户端主动断开）
        if(ret < 0)
        {
            qDebug() << "与服务器断开连接" <<Qt::endl;
            net->disconnect();      // 清理连接状态
            return NULL;            // 退出线程
        }
        //接收到服务器端发送的EOF 说明服务器已被关闭
        // ret == 0: 收到EOF，服务器端关闭了连接
        else if(ret == 0)
        {
            qDebug() << "服务器已关闭..." << Qt::endl;
            net->disconnect();      // 清理连接状态
            return NULL;            // 退出线程
        }

        // 成功接收到数据，打印调试信息
        qDebug() << "recv msg:" << msg << Qt::endl;

        // 调用消息处理函数，解析消息并存入队列
        // 上层应用通过get_msg()从队列中读取消息
        net->msg_handle(msg);           //处理数据(存入消息队列中)
    }
    return NULL;
}
