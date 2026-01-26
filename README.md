# 🎮 五子棋对战平台

基于 **C++/Qt** 的分布式实时同步五子棋对战平台，支持本地对战和网络对战两种模式。

![Language](https://img.shields.io/badge/Language-C++-blue)
![Framework](https://img.shields.io/badge/Framework-Qt%206.5-green)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey)

---

## ✨ 功能特性

- 🎯 **本地对战**：双人同机对战，支持悔棋、重新开始
- 🌐 **网络对战**：基于 TCP 的实时在线对战
- 🏠 **房间系统**：创建/加入房间，支持多房间同时对战
- 🎨 **图形界面**：基于 Qt 的精美 UI 界面
- ⚡ **实时同步**：落子信息实时同步，无延迟体验
- 💬 **游戏内聊天**：对战中可发送消息
- 🔄 **悔棋请求**：网络对战支持发起悔棋请求

---

## 📁 项目结构

```
Code/
├── client/                    # 客户端 (Qt 应用)
│   ├── main.cpp              # 程序入口
│   ├── menu.cpp/h            # 主菜单界面
│   ├── gamewin.cpp/h         # 本地对战界面
│   ├── internet_game.cpp/h   # 网络对战界面
│   ├── client_net.cpp/h      # 网络通信模块
│   ├── gobang_game.pro       # Qt 项目文件
│   ├── res.qrc               # 资源文件
│   └── img/                  # 图片资源
│
└── server/                    # 服务器端 (Linux)
    ├── server.cpp            # 服务器主程序
    └── makefile              # 编译脚本
```

---

## 🛠️ 技术架构

### 客户端
| 技术 | 说明 |
|------|------|
| Qt 6.5 | GUI 框架，信号与槽机制 |
| QPainter | 棋盘和棋子绘制 |
| Winsock2 | Windows 网络通信 |
| 多线程 | 独立的消息接收线程 |

### 服务器端
| 技术 | 说明 |
|------|------|
| epoll | I/O 多路复用，高并发处理 |
| TCP | 可靠的消息传输 |
| 非阻塞 Socket | 提升服务器响应能力 |

### 通信协议
| 信号 | 功能 |
|------|------|
| `C:房间名` | 创建房间 |
| `J:房间号` | 加入房间 |
| `R` | 刷新房间列表 |
| `E` | 退出房间 |
| `U` | 更新准备状态 |
| `OMxy` | 落子信息 (x, y 坐标) |

---

## 🚀 快速开始

### 环境要求
- Qt 6.5 或更高版本
- MinGW / MSVC 编译器 (客户端)
- GCC (服务器端)
- Linux 服务器 (运行服务端)

### 编译客户端

```bash
# 克隆仓库
git clone https://github.com/kiyobee-152/-Cpp-Qt-
cd -Cpp-Qt-/Code/client

# 使用 qmake 编译
qmake gobang_game.pro
make

# 或使用 Qt Creator 打开 .pro 文件直接编译
```

### 编译服务器

```bash
cd -Cpp-Qt-/Code/server

# 编译
make

# 运行 (默认端口 4396)
./server

# 指定端口运行
./server 8080
```

### 配置服务器地址

修改 `client_net.cpp` 中的服务器 IP：

```cpp
client_addr.sin_addr.s_addr = inet_addr("你的服务器IP");
client_addr.sin_port = htons(4396);  // 端口号
```

---

## 🎮 使用说明

### 本地对战
1. 启动客户端
2. 点击「本地对战」
3. 黑棋先手，双方轮流落子
4. 五子连珠即获胜

### 网络对战
1. 确保服务器已启动
2. 点击「网络对战」→ 连接服务器
3. 创建房间或加入已有房间
4. 双方准备后开始对战

---

## 🎯 核心算法

### 胜负判断
采用**四方向扫描算法**，检查落子点的四个方向：

```
    ↖ ↑ ↗
     \|/
   ← ─●─ →    检查：垂直、水平、两条对角线
     /|\
    ↙ ↓ ↘
```

### 点击检测
为每个交叉点设置**矩形点击区域**，使用 `QRect::intersects()` 判断点击位置：

```cpp
// 约 41×41 像素的点击容错范围
QRect(centerX - 20, centerY - 20, 41, 41)
```

---

## 📸 界面预览

```
┌────────────────────────────────────┐
│           五子棋对战平台            │
│                                    │
│    ┌──────────┐  ┌──────────┐     │
│    │ 本地对战  │  │ 网络对战  │     │
│    └──────────┘  └──────────┘     │
│                                    │
│    ┌──────────┐  ┌──────────┐     │
│    │   关于   │  │   退出   │     │
│    └──────────┘  └──────────┘     │
└────────────────────────────────────┘
```

---

## 🤝 贡献指南

欢迎贡献代码！请按以下步骤：

1. Fork 本仓库
2. 创建特性分支：`git checkout -b feature/YourFeature`
3. 提交更改：`git commit -m "Add YourFeature"`
4. 推送分支：`git push origin feature/YourFeature`
5. 提交 Pull Request

---

## 📄 开源协议

本项目采用 [MIT License](LICENSE) 开源协议。

---

本项目开源供大家学习参考，如有问题欢迎提 Issue！

---

## 🌟 Star History

如果这个项目对你有帮助，欢迎点个 ⭐ Star！
