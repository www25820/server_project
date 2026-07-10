#include <iostream>
#include <string>
#include <winsock2.h>
#include <windows.h>

int main() {
    // 控制台 GBK 编码（中文 Windows 原生支持）
    system("chcp 936 > nul");
    SetConsoleOutputCP(936);
    SetConsoleCP(936);

    // 1. 初始化 WinSock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WinSock 初始化失败！" << std::endl;
        system("pause");
        return 1;
    }

    // 2. 创建 socket
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "创建 socket 失败！" << std::endl;
        WSACleanup();
        system("pause");
        return 1;
    }

    // 3. 设置服务端地址
    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    std::cout << "正在连接服务器..." << std::endl;

    // 4. 连接服务端
    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "连接服务器失败！请确认服务器已启动（先运行 server.exe）。" << std::endl;
        closesocket(client_socket);
        WSACleanup();
        system("pause");
        return 1;
    }

    std::cout << "已连接到服务器！" << std::endl;
    while(true){
    // ===== 收发数据的部分 =====
    // 5a. 发消息给服务器
    std::string msg;
    getline(std::cin, msg);
    if(msg == "/quit") {
        std::cout << "客户端退出。" << std::endl;
        break;
    }
    if (msg.empty()) continue;  // 不发空消息
    send(client_socket, msg.c_str(), msg.length(), 0);
    std::cout << "发送: " << msg << std::endl;

    // 5b. 接收服务器回复
    char buf[1024] = {};
    int len = recv(client_socket, buf, sizeof(buf) - 1, 0);
    if (len > 0) {
        buf[len] = '\0';
        std::cout << "服务器回复: " << buf << std::endl;
    }
}
    // ==========================

    // 6. 清理
    closesocket(client_socket);
    WSACleanup();

    std::cout << "客户端关闭。" << std::endl;
    system("pause");
    return 0;
}
