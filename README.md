# TCP Chat Server

基于 WinSock2 的多线程 TCP 聊天室，支持多客户端并发连接、消息广播和优雅退出。

## 功能

- **多客户端并发**：`std::thread` 为每个连接创建独立线程，互不阻塞
- **消息广播**：任一客户端发送消息，服务器转发给所有在线客户端
- **上下线通知**：客户端加入/离开时自动广播提醒
- **收发分离**：客户端双线程分别处理发送和接收，避免互相阻塞
- **线程安全**：`std::mutex` 保护共享客户端列表，防止并发修改导致数据竞争
- **TCP 粘包处理**：基于 `\n` 分帧的 `recv_line` 逐字节读取，解决粘包问题
- **端口重用**：`SO_REUSEADDR` 允许重启后立即绑定端口，无需等待 TIME_WAIT
- **空消息过滤**：过滤空输入和纯换行，避免无效广播

## 环境要求

- 操作系统：Windows
- 编译器：MinGW-w64 (g++) 或 MSVC
- 依赖：`ws2_32` 库

## 编译

```bash
# 服务器
g++ server.cpp -o server.exe -lws2_32 -Wall -Wextra -finput-charset=UTF-8 -fexec-charset=GBK

# 客户端
g++ client.cpp -o client.exe -lws2_32 -Wall -Wextra -finput-charset=UTF-8 -fexec-charset=GBK
```

## 运行

1. 启动服务器：

   ```bash
   ./server.exe
   ```

2. 启动一个或多个客户端：

   ```bash
   ./client.exe
   ```

3. 在任意客户端输入消息，其他客户端实时接收
4. 输入 `/quit` 退出客户端

## 项目结构

```text
server_project/
├── server.cpp                  # 服务端：多线程 accept + 广播 + recv_line
├── client.cpp                  # 客户端：双线程收发分离
├── .gitignore
└── README.md
```

## 要点

| 概念 | 说明 |
| :--- | :--- |
| TCP 粘包 | TCP 是字节流，无消息边界；`\n` 分帧逐字节读取解决粘包 |
| `recv_line` | 每次读取 1 字节直到遇到 `\n`，保证每次拿到完整的一行消息 |
| 收发分离 | `recv` 是阻塞操作，单线程轮询会导致发送卡顿；双线程各自负责一端 |
| `std::mutex` + 客户端列表 | `broadcast` 遍历列表持锁，`erase` 等锁后执行，避免死锁和并发冲突 |
| `SO_REUSEADDR` | 允许绑定处于 TIME_WAIT 状态的端口，调试期间频繁重启无需等待 |
| 锁的作用域 | `{}` 限定锁生命周期，broadcast 内部不再加锁，防止递归锁死锁 |

## 参考资料

- [Microsoft: WinSock2 API](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/)
- [cppreference: std::thread](https://en.cppreference.com/w/cpp/thread/thread)
- [TCP 粘包/拆包原理](https://en.wikipedia.org/wiki/IP_fragmentation)
