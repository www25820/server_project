/**
 * 多线程聊天室 —— 服务端
 * 每来一个客户端开一个线程，收到消息广播给所有人
 */
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <winsock2.h>
#include <windows.h>

// ===== 共享数据（多线程访问，需 mutex 保护）=====
std::vector<SOCKET> clients;      // 所有在线客户端的 socket
std::mutex clients_mutex;         // 保护 clients 的锁

// ===== 广播：把消息发给所有人（跳过发送者自己）=====
void broadcast(const std::string& msg, SOCKET sender = INVALID_SOCKET) {
    std::lock_guard<std::mutex> lock(clients_mutex);  // RAII 锁：离开作用域自动解锁
    for (SOCKET client : clients) {
        if (client != sender) {                       // 不发给自己
            send(client, msg.c_str(), msg.length(), 0);
        }
    }
}

// ===== 处理一个客户端（在独立线程中运行）=====
void handle_client(SOCKET client_socket) {
    // 1. 接收用户名（约定：客户端连接后第一条消息就是昵称）
    char name_buf[64] = {};
    int name_len = recv(client_socket, name_buf, sizeof(name_buf) - 1, 0);
    std::string username = "匿名";
    if (name_len > 0) {
        name_buf[name_len] = '\0';
        username = name_buf;
    }

    // 2. 加入在线列表 + 广播上线通知
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(client_socket);
    }  // 锁在这里释放（离开花括号），只锁必要的范围
    std::cout << "[上线] " << username << std::endl;
    broadcast("[系统] " + username + " 进入了聊天室\n", client_socket);

    // 3. 循环：收消息 → 广播 → 收消息 → 广播 → ...
    while (true) {
        char buf[1024] = {};
        int len = recv(client_socket, buf, sizeof(buf) - 1, 0);
        if (len <= 0) break;  // 客户端断开（recv 返回 0）或出错（返回 -1）

        buf[len] = '\0';  // 手动加字符串结束符，不然 cout 会打印垃圾
        std::string msg = username + ": " + buf;
        std::cout << msg << std::endl;
        broadcast(msg + "\n", client_socket);  // 带换行符广播
    }

    // 4. 离开列表 + 广播下线通知
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        // erase-remove 惯用法：从 vector 中删除指定元素
        clients.erase(
            std::remove(clients.begin(), clients.end(), client_socket),
            clients.end()
        );
    }
    std::cout << "[下线] " << username << std::endl;
    broadcast("[系统] " + username + " 离开了聊天室\n", client_socket);
    closesocket(client_socket);
}

// ===== 主函数：初始化 → 循环 accept → 开线程 =====
int main() {
    // 控制台 GBK 编码（中文 Windows 不乱码）
    system("chcp 936 > nul");
    SetConsoleOutputCP(936);
    SetConsoleCP(936);

    // 1. 初始化 WinSock（Windows 网络编程必须第一步）
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WinSock 初始化失败！" << std::endl;
        system("pause");
        return 1;
    }

    // 2. 创建 TCP socket（AF_INET=IPv4, SOCK_STREAM=TCP）
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "创建 socket 失败！" << std::endl;
        WSACleanup();
        system("pause");
        return 1;
    }

    // 端口重用：重启服务器不用等端口释放（SO_REUSEADDR）
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    // 3. 绑定 127.0.0.1:8080（htons 把端口转成网络字节序）
    sockaddr_in addr = {};  // = {} 零初始化，避免随机值
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(server_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "绑定端口 8080 失败！" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        system("pause");
        return 1;
    }

    // 4. 开始监听（SOMAXCONN = 系统允许最大排队数）
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "监听失败！" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        system("pause");
        return 1;
    }

    std::cout << "服务器已启动，等待客户端连接..." << std::endl;

    // 5. 死循环：accept 阻塞等连接 → 来一个就开一个线程 → 继续等下一个
    while (true) {
        SOCKET client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "接受客户端连接失败！" << std::endl;
            continue;  // 一次失败不退出，继续等
        }
        std::cout << "[连接] 新客户端" << std::endl;
        std::thread(handle_client, client_socket).detach();  // detach: 线程独立运行
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
