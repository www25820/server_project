# TCP 服务器 & 客户端

我的第一个 C++ 网络编程项目 —— 基于 WinSock2 的 TCP 服务器和客户端。

## 功能

- 服务器监听 `127.0.0.1:8080`，支持**多客户端同时在线**
- 客户端持续发送消息 → 服务器接收并回复
- `std::thread` 多线程并发，每个客户端独立线程处理
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
   ```
   ./server.exe
   ```
2. 再在另一个终端窗口启动客户端：
   ```
   ./client.exe
   ```

## 项目结构

```
server_project/
├── server.cpp                # 服务端代码
├── client.cpp                # 客户端代码
├── study/                    # 学习注释版（逐行讲解）
│   ├── server_annotated.cpp
│   └── client_annotated.cpp
├── .gitignore                # Git 忽略规则
└── README.md                 # 项目说明
```

## 学习路线

- [x] 基本的 TCP 连接（server ↔ client）
- [x] 发送和接收消息（`send` / `recv`）
- [x] 循环处理多个客户端（`while` + `std::thread`）
- [x] 持续聊天（循环收发 + `/quit` 退出 + 空消息过滤）
- [ ] 实现简单聊天室（广播消息 + 共享列表 + mutex）

## 作者

一个正在学习 C++ 网络编程的新手 😊
