# TCP 服务器 & 客户端

我的第一个 C++ 网络编程项目 —— 基于 WinSock2 的 TCP 服务器和客户端。

## 功能

- 服务器监听 `127.0.0.1:8080`，支持**多客户端同时在线**
- **广播聊天室**：一人发消息，所有人可见
- 上下线通知：有人进入/离开时广播提醒
- `std::thread` 多线程并发，每个客户端独立线程处理
- 客户端双线程收发分离，发送和接收互不阻塞
- `std::mutex` 保护共享客户端列表
- `SO_REUSEADDR` 端口重用，重启无需等待
- 输入 `/quit` 退出客户端，空消息自动过滤

## 环境要求

- Windows 系统
- MinGW-w64 (g++) 或 Visual Studio
- 需要链接 `ws2_32` 库

## 编译

### 使用 g++ (MinGW)

```bash
# 编译服务器
g++ server.cpp -o server.exe -lws2_32 -Wall -Wextra -finput-charset=UTF-8 -fexec-charset=GBK

# 编译客户端
g++ client.cpp -o client.exe -lws2_32 -Wall -Wextra -finput-charset=UTF-8 -fexec-charset=GBK
```

### 使用 VS Code

按 `Ctrl+Shift+B` 选择编译任务（需要先修改 `.vscode/tasks.json` 中的 g++ 路径）。

## 运行

1. 先在一个终端窗口启动服务器：
   ```bash
   ./server.exe
   ```
2. 再开多个终端窗口启动客户端（可以开 2-3 个测试聊天）：
   ```bash
   ./client.exe
   ```
3. 在其中一个客户端输入消息，其他客户端都能看到

## 项目结构

```
server_project/
├── server.cpp                # 服务端代码（多线程 + 按行读）
├── client.cpp                # 客户端代码（双线程收发分离）
├── .gitignore                # Git 忽略规则
└── README.md                 # 项目说明
```

## 学习路线

- [x] 基本的 TCP 连接（server ↔ client）
- [x] 发送和接收消息（`send` / `recv`）
- [x] 循环处理多个客户端（`while` + `std::thread`）
- [x] 持续聊天（循环收发 + `/quit` 退出 + 空消息过滤）
- [x] 实现简单聊天室（广播消息 + 共享列表 + mutex + 收发分离）
- [x] TCP 粘包处理（`\n` 分隔 + `recv_line` 逐字节读取）

## 作者

一个正在学习 C++ 网络编程的新手 😊
