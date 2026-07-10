#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <windows.h>

void recv_thread(SOCKET client_socket) {
    while (true) {
        char buf[1024] = {};
        int len = recv(client_socket, buf, sizeof(buf) - 1, 0);
        if (len <= 0) {
            std::cout << "\n[提示] 与服务器断开连接。" << std::endl;
            break;
        }
        buf[len] = '\0';
        std::cout << buf << std::endl;
    }
}

int main() {
    system("chcp 936 > nul");
    SetConsoleOutputCP(936);
    SetConsoleCP(936);

    // ① 先输入用户名
    std::string username;
    std::cout << "请输入昵称: ";
    std::getline(std::cin, username);
    if (username.empty()) username = "匿名";

    // ② 初始化 WinSock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WinSock 初始化失败！" << std::endl;
        system("pause");
        return 1;
    }

    // ③ 创建 socket
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "创建 socket 失败！" << std::endl;
        WSACleanup();
        system("pause");
        return 1;
    }

    // ④ 连接
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

    // ⑤ 先发用户名
    send(client_socket, username.c_str(), username.size(), 0);

    // ⑥ 启动接收线程
    std::thread t(recv_thread, client_socket);
    t.detach();

    std::cout << "========================================" << std::endl;
    std::cout << "  已进入聊天室！输入 /quit 退出" << std::endl;
    std::cout << "========================================" << std::endl;

    // ⑦ 发送循环
    while (true) {
        std::string msg;
        std::getline(std::cin, msg);

        if (msg == "/quit") break;
        if (msg.empty()) continue;

        msg += "\n";
        send(client_socket, msg.c_str(), msg.size(), 0);
    }

    // ⑧ 清理
    closesocket(client_socket);
    WSACleanup();
    std::cout << "已退出聊天室。" << std::endl;
    std::cin.get();
    return 0;
}
