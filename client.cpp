/**
 * 多线程聊天室 —— 客户端
 * 双线程：主线程只管发送，接收线程只管收消息并打印
 */
#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <windows.h>

// ===== 接收线程：后台一直等服务器消息，收到立刻打印 =====
void recv_thread(SOCKET client_socket) {
    while (true) {
        char buf[1024] = {};
        int len = recv(client_socket, buf, sizeof(buf) - 1, 0);  // 阻塞：没消息就一直等
        if (len <= 0) {
            std::cout << "\n[提示] 与服务器断开连接。" << std::endl;
            break;
        }
        buf[len] = '\0';  // 手动加字符串结束符
        std::cout << buf << std::endl;
    }
}

// ===== 主函数：连接 → 发用户名 → 开接收线程 → 循环发送 =====
int main() {
    // 控制台 GBK 编码
    system("chcp 936 > nul");
    SetConsoleOutputCP(936);
    SetConsoleCP(936);

    // 1. 输入昵称
    std::string username;
    std::cout << "请输入昵称: ";
    std::getline(std::cin, username);  // getline 支持空格，cin >> 不行
    if (username.empty()) username = "匿名";

    // 2. 初始化 WinSock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WinSock 初始化失败！" << std::endl;
        system("pause");
        return 1;
    }

    // 3. 创建 TCP socket
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "创建 socket 失败！" << std::endl;
        WSACleanup();
        system("pause");
        return 1;
    }

    // 4. 连接服务器 127.0.0.1:8080
    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    std::cout << "正在连接服务器..." << std::endl;

    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "连接服务器失败！请确认服务器已启动（先运行 server.exe）。" << std::endl;
        closesocket(client_socket);
        WSACleanup();
        system("pause");
        return 1;
    }

    // 5. 先发用户名（加 \n，服务端按行读取）
    std::string name_line = username + "\n";
    send(client_socket, name_line.c_str(), name_line.size(), 0);

    // 6. 启动接收线程（收发分离的关键）
    std::thread t(recv_thread, client_socket);
    t.detach();  // 独立运行，不阻塞主线程

    std::cout << "========================================" << std::endl;
    std::cout << "  已进入聊天室！输入 /quit 退出" << std::endl;
    std::cout << "========================================" << std::endl;

    // 7. 主循环：只负责读输入 → 发送
    while (true) {
        std::string msg;
        std::getline(std::cin, msg);

        if (msg == "/quit") break;         // 退出命令
        if (msg.empty()) continue;         // 空行跳过，不发（发了会导致双方卡死）

        msg += "\n";                       // 加换行符作为消息分隔
        send(client_socket, msg.c_str(), msg.size(), 0);
    }

    // 8. 清理
    closesocket(client_socket);
    WSACleanup();
    std::cout << "已退出聊天室。" << std::endl;
    std::cin.get();
    return 0;
}
