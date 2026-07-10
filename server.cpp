#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <winsock2.h>
#include <windows.h>

std::vector<SOCKET> clients;
std::mutex clients_mutex;

void broadcast(const std::string& msg, SOCKET sender = INVALID_SOCKET) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (SOCKET client : clients) {
        if (client != sender) {
            send(client, msg.c_str(), msg.length(), 0);
        }
    }
}

void handle_client(SOCKET client_socket) {
    // ① 先收用户名
    char name_buf[64] = {};
    int name_len = recv(client_socket, name_buf, sizeof(name_buf) - 1, 0);
    std::string username = "匿名";
    if (name_len > 0) {
        name_buf[name_len] = '\0';
        username = name_buf;
    }

    // ② 加入在线列表 + 通知
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(client_socket);
    }
    std::cout << "[上线] " << username << std::endl;
    broadcast("[系统] " + username + " 进入了聊天室\n", client_socket);

    // ③ 循环接收并广播
    while (true) {
        char buf[1024] = {};
        int len = recv(client_socket, buf, sizeof(buf) - 1, 0);
        if (len <= 0) break;

        buf[len] = '\0';
        std::string msg = username + ": " + buf;
        std::cout << msg << std::endl;
        broadcast(msg + "\n", client_socket);
    }

    // ④ 离开列表 + 通知
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(
            std::remove(clients.begin(), clients.end(), client_socket),
            clients.end()
        );
    }
    std::cout << "[下线] " << username << std::endl;
    broadcast("[系统] " + username + " 离开了聊天室\n", client_socket);
    closesocket(client_socket);
}

int main() {
    system("chcp 936 > nul");
    SetConsoleOutputCP(936);
    SetConsoleCP(936);

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WinSock 初始化失败！" << std::endl;
        system("pause");
        return 1;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "创建 socket 失败！" << std::endl;
        WSACleanup();
        system("pause");
        return 1;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in addr = {};
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

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "监听失败！" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        system("pause");
        return 1;
    }

    std::cout << "服务器已启动，等待客户端连接..." << std::endl;

    while (true) {
        SOCKET client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "接受客户端连接失败！" << std::endl;
            continue;
        }
        std::cout << "[连接] 新客户端" << std::endl;
        std::thread(handle_client, client_socket).detach();
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
