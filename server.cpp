#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <thread>
#include<vector>
#include <mutex>
#include <algorithm>
std::vector<SOCKET> clients;
std::mutex clients_mutex;
void  broadcast(const std::string&msg,SOCKET sender=INVALID_SOCKET){//广播消息
    std::lock_guard<std::mutex> lock(clients_mutex);// 锁定互斥量，确保线程安全
    for(auto client:clients){
        if(client != sender){// 不向发送者发送消息
            send(client,msg.c_str(),msg.length(),0);
        }
    }
}
void handle_client(SOCKET client_socket) {
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(client_socket);
    }// 将客户端 socket 添加到列表中

    // 广播消息，通知其他客户端
    broadcast("[系统]有人进入了聊天室\n",client_socket);

    while(true) {
    char buf[1024] = {};
    int len = recv(client_socket, buf, sizeof(buf) - 1, 0);
    if (len > 0) {
        buf[len] = '\0';
        std::cout << "客户端说: " << buf << std::endl;
    }
    else{
        std::cout << "客户端已断开连接。" << std::endl;
        break;
    }
    std::string msg(buf);
    broadcast(msg+"\n", client_socket);//广播消息给其他客户端
    }
    {std::lock_guard<std::mutex> lock(clients_mutex);
    clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());}
    broadcast("[系统]有人离开了聊天室\n",client_socket);
    //  关闭客户端 socket
    closesocket(client_socket);
    
}
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
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "创建 socket 失败！" << std::endl;
        WSACleanup();
        system("pause");
        return 1;
    }

    // 端口重用
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    // 3. 绑定地址和端口
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(server_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "绑定端口 8080 失败！端口可能已被占用。" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        system("pause");
        return 1;
    }

    // 4. 开始监听
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "监听失败！" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        system("pause");
        return 1;
    }

    std::cout << "服务器已启动，等待客户端连接..." << std::endl;

    // 5. 等待客户端连接
    while(true){
    SOCKET client_socket = accept(server_socket, nullptr, nullptr);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "接受客户端连接失败！" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        system("pause");
        return 1;
    }

    std::cout << "有客户端连进来了！" << std::endl;
    std::thread client_thread(handle_client, client_socket);
    client_thread.detach();

    }
}
