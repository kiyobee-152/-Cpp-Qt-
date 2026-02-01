/**
 * @file server.cpp
 * @brief 五子棋对战平台服务器端实现
 * 
 * 本文件实现了五子棋网络对战的服务器端核心功能，包括：
 * - 基于epoll的高并发网络I/O模型
 * - 客户端连接管理（连接、断开、状态维护）
 * - 房间系统（创建、加入、退出、列表刷新）
 * - 游戏消息转发（落子、聊天、悔棋、认输等）
 * - 准备状态和先后手选择的同步
 * 
 * 运行环境：Linux系统
 * 编译命令：g++ server.cpp -o server
 * 启动方式：./server [端口号]  (默认端口4396)
 */

/* ==================== 头文件包含 ==================== */

// C标准库头文件
#include<stdio.h>       // 标准输入输出（printf, sprintf等）
#include<string.h>      // 字符串操作（memset, strcmp, strlen等）
#include<stdlib.h>      // 标准库函数（atoi等）
#include<assert.h>      // 断言（用于调试）

// Linux网络编程头文件
#include<arpa/inet.h>   // IP地址转换（inet_ntoa, htons等）
#include<unistd.h>      // UNIX标准函数（read, write, close等）
#include<sys/socket.h>  // 套接字API（socket, bind, listen, accept等）
#include<sys/epoll.h>   // epoll多路复用（epoll_create, epoll_ctl, epoll_wait）
#include<fcntl.h>       // 文件控制（open, O_RDONLY等）
#include<error.h>       // 错误处理

// C++ STL头文件
#include<iostream>      // 输入输出流
#include<vector>        // 动态数组
#include<algorithm>     // 算法（remove等）
#include<map>           // 关联容器（哈希映射）
#include<queue>         // 队列（未使用）


using namespace std;

/**
 * @brief 消息缓冲区大小常量
 * 
 * 定义单次消息传输的最大字节数
 */
#define msg_size 1024

/* ==================== 函数前向声明 ==================== */

/**
 * @brief 处理客户端刷新房间列表请求
 * @param client_fd 发起请求的客户端套接字
 */
void R_signal(int client_fd);//处理客户端刷新战局的请求

/**
 * @brief 处理客户端创建房间请求
 * @param msg 包含房间名的消息字符串
 * @param fd 发起请求的客户端套接字
 */
void C_signal(char* msg,int fd);//处理客户端创建房间的请求

/**
 * @brief 处理客户端退出房间请求
 * @param client_fd 发起请求的客户端套接字
 */
void E_signal(int client_fd);//处理客户端退出房间的请求

/**
 * @brief 处理客户端加入房间请求
 * @param fd 发起请求的客户端套接字
 * @param msg 包含目标房间信息的消息字符串
 */
void J_signal(int fd,char* msg);//处理客户端加入房间的请求

/**
 * @brief 处理客户端更新对手状态请求
 * @param fd 发起请求的客户端套接字
 */
void U_signal(int fd);//处理客户端更新对手准备状态的请求


/**
 * @brief 初始化服务器套接字和地址结构
 * @param server_addr 服务器地址结构体引用（输出参数）
 * @param server_fd 服务器套接字引用（输出参数）
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * 
 * 初始化流程：
 * 1. 创建非阻塞TCP套接字
 * 2. 配置服务器地址（支持命令行指定端口，默认4396）
 * 3. 绑定到所有可用网络接口（INADDR_ANY）
 */
void initialization_server(struct sockaddr_in& server_addr,int &server_fd,int argc,char *argv[])
{   
    // 创建TCP套接字
    // PF_INET: IPv4协议族
    // SOCK_STREAM: 流式套接字（TCP）
    // SOCK_NONBLOCK: 非阻塞模式（配合epoll使用）
    // IPPROTO_TCP: TCP协议
    server_fd=socket(PF_INET,SOCK_STREAM|SOCK_NONBLOCK,IPPROTO_TCP);
    
    // 设置地址族为IPv4
    server_addr.sin_family=AF_INET;
    
    // 根据命令行参数设置端口号
    switch(argc)
    {
        case 2:     // 用户指定了端口号
        {
            // 将字符串端口转换为整数，再转换为网络字节序
            server_addr.sin_port=htons(atoi(argv[1]));
            // 绑定到所有可用网络接口
            server_addr.sin_addr.s_addr=INADDR_ANY;
        }break;
        default:    // 使用默认端口4396
        {
            server_addr.sin_port=htons(4396);
            server_addr.sin_addr.s_addr=INADDR_ANY;
        }
    }
}

/**
 * @brief 错误信息输出函数
 * @param msg 错误消息字符串
 * 
 * 统一的错误输出格式，便于调试
 */
void Error_msg(string msg)
{
    cout<<"Error_msg:"<<msg<<endl;
}

/* ==================== 数据结构定义 ==================== */

/**
 * @brief 客户端信息结构体
 * 
 * 存储每个已连接客户端的游戏状态信息
 */
struct client_information
{
	int opponent_fd;    // 对手的套接字描述符（0表示没有对手）
	bool prepare;       // 准备状态（true:已准备, false:未准备）
    bool master;        // 是否为房间创建者（房主）
    int room_num;       // 所在房间的索引号（-1表示不在任何房间）
    
    /**
     * @brief 默认构造函数
     * 
     * 初始化为：无对手、未准备、非房主、不在房间
     */
	client_information() :opponent_fd(0), prepare(0),room_num(-1),master(false){}
};

/**
 * @brief 房间信息结构体
 * 
 * 存储每个游戏房间的基本信息
 */
struct room_information
{   
    int client_fd;      // 房间中客人（加入者）的套接字（-1表示空位）
    string room_name;   // 房间名称（由创建者设定）
    int master_fd;      // 房间中主人（创建者）的套接字
    
    /**
     * @brief 带参数构造函数
     * @param name 房间名称
     * @param fd 房主的套接字
     */
    room_information(string name,int fd):room_name(name),master_fd(fd),client_fd(-1){}
};

/* ==================== 全局数据容器 ==================== */

/**
 * @brief 客户端信息映射表
 * 
 * 键: 客户端套接字描述符
 * 值: 该客户端的游戏状态信息
 * 用于快速查询任意客户端的状态
 */
map<int,client_information>hash_client;//每一个套接字对应一个客户端信息

/**
 * @brief 客户端地址映射表
 * 
 * 键: 客户端套接字描述符
 * 值: 该客户端的网络地址信息（IP、端口等）
 * 用于获取客户端的IP地址等信息
 */
map<int,struct sockaddr_in>client_addrs;//每一个套接字对应一个客户端的ip地址等信息

/**
 * @brief 房间列表
 * 
 * 存储所有已创建的房间
 * 索引即为房间号
 */
vector<room_information>rooms;//房间

/**
 * @brief 所有已连接客户端的套接字列表
 * 
 * 用于统计在线人数和遍历客户端
 */
vector<int>client_fds;//所有客户端套接字

/* ==================== 主函数 ==================== */

/**
 * @brief 服务器主函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组（argv[1]可指定端口号）
 * @return int 程序退出码
 * 
 * 服务器工作流程：
 * 1. 初始化服务器套接字
 * 2. 设置套接字选项并绑定端口
 * 3. 开始监听连接
 * 4. 创建epoll实例并注册服务器套接字
 * 5. 进入事件循环，处理连接和消息
 */
int main(int argc,char* argv[])
{   
    // 打开空设备文件，用于处理文件描述符耗尽的情况
    // 这是一种优雅处理EMFILE错误的技巧
    int idle_fd=open("/dev/null",O_RDONLY|O_CLOEXEC);
    
    // 服务器和客户端套接字
    int server_fd,client_fd;
    
    // 服务器和客户端地址结构
    struct sockaddr_in server_addr,client_addr;

    socklen_t client_sz;    // 客户端地址结构大小
    int ret=0;;             // 函数返回值
    char msg[msg_size];     // 消息缓冲区
    
    // 初始化服务器套接字和地址
    initialization_server(server_addr,server_fd,argc,argv);

    // 设置套接字选项：允许地址重用
    // 解决服务器重启时"Address already in use"问题
    int optset=1;
    ret=setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&optset,sizeof(optset));assert(ret==0);
    
    // 绑定服务器地址到套接字
    ret=bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr));assert(ret==0);
    
    // 开始监听，等待队列长度为5
    ret=listen(server_fd,5);assert(ret==0);

    // ========== epoll初始化 ==========
    
    struct epoll_event event;               // 单个epoll事件
    vector<struct epoll_event>events(16);   // 事件数组，初始容量16
    int epoll_fd;                           // epoll实例描述符

    // 创建epoll实例（参数在Linux 2.6.8后被忽略，但必须大于0）
    epoll_fd=epoll_create(5555);
    
    // 配置服务器套接字的epoll事件
    event.data.fd=server_fd;    // 关联服务器套接字
    event.events=EPOLLIN;       // 监听可读事件（有新连接时触发）

    // 将服务器套接字添加到epoll监控
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,server_fd,&event);


    // ========== 主事件循环 ==========
    //使用EPOLL模型
    while(1)
    {
        // 等待epoll事件
        // 参数：epoll描述符、事件数组、数组大小、超时时间（-1表示永久阻塞）
        int event_cnt=epoll_wait(epoll_fd,&*events.begin(),static_cast<int>(events.size()),-1);
        
        // 错误处理
        if(event_cnt==-1)
        {
            // 被信号中断，继续等待
            if(errno==EINTR)
                continue;
            Error_msg("epoll_wait");
        }
        
        // 没有事件，继续等待
        if(event_cnt==0)
            continue;
        
        // 动态扩容：如果事件数组满了，扩大一倍
        if((size_t)event_cnt==events.size())
            events.resize(events.size()*2);
        
        // 遍历所有触发的事件
        for(int i=0;i<event_cnt;i++)
        {
            // ========== 处理新客户端连接 ==========
            if(events[i].data.fd==server_fd)
            {
                client_sz=sizeof(client_sz);
                
                // accept4: 接受连接并设置非阻塞标志
                client_fd=accept4(server_fd,(struct sockaddr*)&client_addr,&client_sz,O_NONBLOCK);
                //client_fd=accept(server_fd,(struct sockaddr*)&client_addr,&client_sz);

                // 接受连接失败处理
                if(client_fd==-1)
                {
                    // EMFILE: 文件描述符耗尽
                    // 使用预留的idle_fd优雅处理
                    if(errno==EMFILE)
                    {
                        close(idle_fd);                             // 关闭预留fd
                        idle_fd=accept(server_fd,NULL,NULL);        // 接受连接（会立即获得fd）
                        close(idle_fd);                             // 关闭该连接
                        idle_fd=open("/dev/null",O_RDONLY|O_NONBLOCK);  // 重新打开预留fd
                        continue;
                    }
                    else
                    Error_msg("accept4?");
                }
                
                // 打印新连接信息
                printf("[%d][Client]<IP:%s><PT:%d><***CONNECT***>\n",__LINE__,inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
                
                // 保存客户端地址信息
                client_addrs[client_fd]=client_addr;
                
                // 配置客户端套接字的epoll事件
                event.data.fd=client_fd;
                event.events=EPOLLIN|EPOLLET;   // 可读事件 + 边缘触发模式
                
                // 将客户端套接字添加到epoll监控
                epoll_ctl(epoll_fd,EPOLL_CTL_ADD,client_fd,&event);
                
                // 记录客户端套接字
                client_fds.push_back(client_fd);

                // 为该客户端创建默认信息记录
                hash_client[client_fd];//**************r
            }
            // ========== 处理客户端消息 ==========
            else if(events[i].events&EPOLLIN)
            {
                client_fd=events[i].data.fd;
                
                // 无效套接字，跳过
                if(client_fd<0)
                    continue;
                
                // 清空消息缓冲区并读取数据
                memset(msg,0,sizeof(msg));
                ret=read(client_fd,msg,sizeof(msg));

                // ========== 处理客户端断开连接 ==========
                // ret <= 0: 连接关闭或读取错误
                if(ret<=0)
                {   
                    printf("[%d][CLient]<FD:%d><***CLOSE***>\n",__LINE__,client_fd);
                    
                    // 处理退出房间逻辑
                    E_signal(client_fd);
                    
                    // 关闭套接字
                    close(client_fd);
                    
                    // 从epoll中移除
                    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,client_fd,NULL);

                    // 从客户端列表中移除
                   client_fds.erase(remove(client_fds.begin(),client_fds.end(),client_fd),client_fds.end());
                   continue;
                }
                
                // ========== 处理对战消息（O开头）==========
                // 'O'开头的消息为opponent消息，需要转发给对手
                if(msg[0]=='O')//当消息头字母为O时候，代表为opponent消息，此类消息直接原地传回对手客户端处理
                switch(msg[1])
                {
                    case 'M':   // Move: 落子消息
                    {
                        // 转发给对手
                        write(hash_client[client_fd].opponent_fd,msg,strlen(msg));
                    }break;
                    case 'B':   // Back: 悔棋消息
                    {
                        write(hash_client[client_fd].opponent_fd,msg,strlen(msg));
                    }break;
                    case 'N':   // Note: 聊天消息
                    {
                        write(hash_client[client_fd].opponent_fd,msg,strlen(msg));
                    }break;
                    case 'R':   // Run away: 对手退出消息
                    {
                        write(hash_client[client_fd].opponent_fd,msg,strlen(msg));
                    }break;
                    case 'S':   // Surrender: 认输消息
                    {
                        write(hash_client[client_fd].opponent_fd,msg,strlen(msg));
                    }
                }
                
                // ========== 处理准备和先后手消息 ==========
                //以下三个if语句中消息处理分别表示接受到客户端的准备请求（服务器这边会更新准备信息）、原地转发先手并给对手传送后手的消息
                
                // 处理准备/取消准备消息
                if(strcmp(msg,"prepare")==0)
                {
                    // 切换准备状态
                    hash_client[client_fd].prepare=!hash_client[client_fd].prepare;
                    
                    // 检查是否双方都已准备
                    // 条件：己方已准备 && 有对手 && 对手已准备
                    if(hash_client[client_fd].prepare&&hash_client[client_fd].opponent_fd>0&&hash_client[hash_client[client_fd].opponent_fd].prepare)
                    {   
                        // 通知双方游戏开始
                        //printf("[%d]game_start",__LINE__);
                        write(client_fd,"/Zstart",strlen("/Zstart"));
                        write(hash_client[client_fd].opponent_fd,"/Zstart",strlen("/Zstart"));
                    }
                }
                
                // 处理选择黑棋（先手）消息
                if(strcmp(msg,"color1")==0)
                {
                    // 发送者为黑棋（先手）
                    write(client_fd,"c1",strlen("c1"));
                    // 对手为白棋（后手）
                    write(hash_client[client_fd].opponent_fd,"c0",strlen("c0"));
                }
                
                // 处理选择白棋（后手）消息
                if(strcmp(msg,"color0")==0)
                {
                    // 发送者为白棋（后手）
                    write(client_fd,"c0",strlen("c0"));
                    // 对手为黑棋（先手）
                    write(hash_client[client_fd].opponent_fd,"c1",strlen("c1"));
                }
                
                // ========== 处理系统命令消息 ==========
                // 这些消息用于游戏开始前的客户端-服务器交互
                //特殊信息处理，一般是用于游戏开始前的客户端服务端交互
                switch(msg[0])
                {
                    case 'R':R_signal(client_fd);break;     // Refresh: 刷新房间列表
                    case 'C':C_signal(msg,client_fd);break; // Create: 创建房间
                    case 'E':E_signal(client_fd);break;     // Exit: 退出房间
                    case 'J':J_signal(client_fd,msg);break; // Join: 加入房间
                    case 'U':U_signal(client_fd);break;     // Update: 更新对手状态
                    //default:break;
                }
                
                // 调试输出（已注释）
                //printf("[%d][CLient%d]<%d>:%s\n",__LINE__,client_fd,ret,msg);
                //write(client_fd,msg,strlen(msg));
            }
        }
    }
    
    // 清理资源（实际上不会执行到这里，因为是无限循环）
    close(server_fd);
    close(epoll_fd);
    return 0;
}

/* ==================== 消息处理函数实现 ==================== */

/**
 * @brief 处理刷新房间列表请求（R信号）
 * @param client_fd 发起请求的客户端套接字
 * 
 * 响应流程：
 * 1. 统计空闲房间数量（没有客人加入的房间）
 * 2. 发送在线人数和空闲房间数
 * 3. 对每个空闲房间，发送房间名、房主IP、房主FD
 * 
 * 响应数据格式：
 * - /S{在线人数}/S{空闲房间数}
 * - 对于每个空闲房间：/N{房间名}/I{IP地址}/F{套接字FD}
 */
void R_signal(int client_fd)
{
    int sum=0;  // 空闲房间计数

    char msg_[1024];
    
    // 统计空闲房间数量
    // client_fd == -1 表示房间没有客人，即为空闲
    for(auto x:rooms)
    {
        if(x.client_fd==-1)
            sum++;
    }
    
    // 发送在线人数和空闲房间数
    // 格式: /S{在线人数}/S{空闲房间数}
    memset(msg_,0,sizeof(msg_));
    sprintf(msg_,"/S%ld/S%d",client_fds.size(),sum);
    //printf("[%d]%d\n",__LINE__,sum);
    write(client_fd,msg_,strlen(msg_));
    
    // 发送每个空闲房间的详细信息
    for(auto x:rooms)
    {
        if(x.client_fd==-1)     // 只发送空闲房间
        {   
            // 发送房间名
            // 格式: /N{房间名}
            memset(msg_,0,sizeof(msg_));
            sprintf(msg_,"/N%s",x.room_name.c_str());
            //printf("[%d]%s\n",__LINE__,msg_);
            write(client_fd,msg_,strlen(msg_));
            
            // 发送房主IP地址
            // 格式: /I{IP地址}
            memset(msg_,0,sizeof(msg_));
            sprintf(msg_,"/I%s",inet_ntoa(client_addrs[x.master_fd].sin_addr));
            //printf("[%d]%s\n",__LINE__,msg_);
            write(client_fd,msg_,strlen(msg_));

            // 发送房主套接字FD（用于加入房间时标识目标）
            // 格式: /F{套接字FD}
            memset(msg_,0,sizeof(msg_));
            sprintf(msg_,"/F%d",x.master_fd);
            //printf("[%d]%s\n",__LINE__,msg_);
            write(client_fd,msg_,strlen(msg_));
        }
    }
}

/**
 * @brief 处理创建房间请求（C信号）
 * @param msg 消息字符串，格式为 "C:{房间名}"
 * @param fd 创建者的套接字
 * 
 * 处理流程：
 * 1. 从消息中提取房间名（跳过前两个字符"C:"）
 * 2. 创建房间信息对象并添加到房间列表
 * 3. 更新创建者的客户端信息（设置房间号和房主标志）
 */
void C_signal(char* msg,int fd)
{   
    char buf[1024];
    memset(buf,0,sizeof(buf));
    
    // 提取房间名（从msg[2]开始，跳过"C:"前缀）
    for(int i=2;msg[i]!='\0';i++)
        buf[i-2]=msg[i];
    
    //printf("[%d]%s\n",__LINE__,buf);
    
    // 创建房间对象（房间名、房主FD）
    room_information room(buf,fd);
    
    // 添加到房间列表
    rooms.push_back(room);
    
    // 更新创建者的客户端信息
    hash_client[fd].room_num=rooms.size()-1;    // 房间号为列表最后一个索引
    hash_client[fd].master=true;                // 标记为房主
}

/**
 * @brief 处理退出房间请求（E信号）
 * @param fd 退出者的套接字
 * 
 * 退出逻辑（根据退出者身份不同）：
 * 
 * 1. 如果退出者不在任何房间：直接返回
 * 
 * 2. 如果退出者是客人（非房主）：
 *    - 将房间的客人位置设为空（-1）
 *    - 清除房主对该客人的引用
 *    - 重置退出者的客户端信息
 * 
 * 3. 如果退出者是房主：
 *    a. 房间有客人时：客人升级为新房主
 *    b. 房间无客人时：删除整个房间，更新后续房间的索引
 */
void E_signal(int fd)
{   
    char msg[1024];
    memset(msg,0,sizeof(msg));
    
    // 不在任何房间，无需处理
    if(hash_client[fd].room_num==-1)
        return ;
    
    // ===== 情况1: 退出者是客人（非房主）=====
    if(!hash_client[fd].master)
    {
        // 将房间的客人位置设为空
        rooms[hash_client[fd].room_num].client_fd=-1;
        
        // 清除房主对该客人的引用
        // hash_client[fd].room_num=-1;
        // hash_client[fd].prepare=0;
        // hash_client[fd].opponent_fd=0;
        hash_client[hash_client[fd].opponent_fd].opponent_fd=0;
        
        // 可选：通知房主客人已离开（已注释）
        //sprintf(msg,"/O0");
        //write(hash_client[fd].opponent_fd,msg,strlen(msg));
        
        // 重置退出者的客户端信息
        hash_client[fd]=client_information();
        return ;
    }
    // ===== 情况2: 退出者是房主 =====
    else
    {
        // 情况2a: 房间有客人，客人升级为新房主
        if(hash_client[fd].opponent_fd>0)
        {
            int client_fd=hash_client[fd].opponent_fd;  // 获取客人FD
            int room_num=hash_client[fd].room_num;      // 获取房间号
            
            // 清除客人对原房主的引用
            hash_client[client_fd].opponent_fd=0;
            // 客人升级为新房主
            hash_client[client_fd].master=true;
            // 更新房间的房主信息
            rooms[room_num].master_fd=client_fd;
            // 房间客人位置设为空
            rooms[room_num].client_fd=-1;
        }
        // 情况2b: 房间无客人，删除房间
        else
        {
            int r=hash_client[fd].room_num;
            
            // 从房间列表中删除该房间
            rooms.erase(rooms.begin()+r,rooms.begin()+r+1);
            
            // 更新后续房间中所有玩家的房间号（因为索引发生了变化）
            for(int i=r;i<rooms.size();i++)
            {
                // 更新客人的房间号
                if(rooms[i].client_fd>0)
                    hash_client[rooms[i].client_fd].room_num=i;
                // 更新房主的房间号
                hash_client[rooms[i].master_fd].room_num=i;
            }
        }
        
        // 重置退出者的客户��信息
        hash_client[fd]=client_information();
    }
}

/**
 * @brief 处理加入房间请求（J信号）
 * @param fd 加入者的套接字
 * @param msg 消息字符串，格式为 "J{目标房主的FD}"
 * 
 * 加入流程：
 * 1. 从消息中提取目标房主的FD
 * 2. 验证目标房间是否有效且可加入
 * 3. 建立双向的对手引用
 * 4. 更新房间信息
 * 5. 返回成功/失败响应
 * 
 * 失败条件：
 * - 目标FD无效（<=0）
 * - 目标不在任何房间（room_num < 0）
 * - 房间已满（opponent_fd > 0）
 */
void J_signal(int fd,char* msg)
{
    int sum=0;
    
    // 解析目标房主的FD（从msg[1]开始，跳过'J'前缀）
    for(int i=1;msg[i]!='\0';i++)
        sum=sum*10+msg[i]-'0';
    
    // 验证加入条件
    // sum: 目标房主FD
    // hash_client[sum].room_num < 0: 目标不在房间
    // hash_client[sum].opponent_fd > 0: 房间已有人
    if(sum<=0||hash_client[sum].room_num<0||hash_client[sum].opponent_fd>0)
    {
        // 返回错误响应
        write(fd,"/Zerror",strlen("/Zerror"));
        return;
    }
    
    // 建立双向对手引用
    hash_client[sum].opponent_fd=fd;    // 房主的对手设为加入者
    hash_client[fd].opponent_fd=sum;    // 加入者的对手设为房主
    
    // 更新房间信息
    rooms[hash_client[sum].room_num].client_fd=fd;  // 设置房间的客人
    hash_client[fd].room_num=hash_client[sum].room_num; // 设置加入者的房间号
    
    // 返回成功响应
    write(fd,"/Zsuccess",strlen("/Zsuccess"));
}

/**
 * @brief 处理更新对手状态请求（U信号）
 * @param fd 请求者的套接字
 * 
 * 响应格式：
 * /Z{是否有对手(1/0)}/Z{对手准备状态}/Z{对手IP}/Z{对手FD}
 * 
 * 有对手时：返回对手的详细信息
 * 无对手时：返回 /Z0/Z /Z /Z （占位符）
 */
void U_signal(int fd)
{
    char msg[1024];
    memset(msg,0,sizeof(msg));
    
    // 检查是否有对手
    if(hash_client[fd].opponent_fd>0)
    {
        // 有对手：返回对手的详细信息
        // 格式: /Z1/Z{准备状态}/Z{IP地址}/Z{FD}
        sprintf(msg,"/Z1/Z%d/Z%s/Z%d",
            hash_client[hash_client[fd].opponent_fd].prepare,   // 对手准备状态
            inet_ntoa(client_addrs[hash_client[fd].opponent_fd].sin_addr),  // 对手IP
            hash_client[fd].opponent_fd);   // 对手FD
    }
    else
    {
        // 无对手：返回占位符
        sprintf(msg,"/Z0/Z /Z /Z ");
    }
    
    write(fd,msg,strlen(msg));
}
