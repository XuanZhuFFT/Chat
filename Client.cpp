#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <thread>
#include <mutex>

#define BUFFSIZE 2048
#define DEFAULT_PORT 16555 
#define SERVER_ADDR "192.168.101.195"

int main(){
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == client_socket)
    {
        std::cerr << "socket error" << std::endl;
        return -1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEFAULT_PORT);
    inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);

    if (-1 == connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        printf("Connect error(%d): %s\n", errno, strerror(errno));
        return -1;
    }

    char send_buffer[BUFFSIZE] = { 0 };
    char recv_buffer[BUFFSIZE] = { 0 };
    while(true)
    {
        memset(recv_buffer, 0, sizeof(recv_buffer)); // 清空接收缓冲区
        memset(send_buffer, 0, sizeof(send_buffer)); // 清空发送缓冲区
    
        std::cout << "Please input: ";
        fgets(send_buffer, BUFFSIZE, stdin);
        send_buffer[strcspn(send_buffer, "\n")] = 0;  // 去掉换行符
    
        // 若用户直接回车，跳过
        if (strlen(send_buffer) == 0) continue;
    
        send(client_socket, send_buffer, strlen(send_buffer), 0);
    
        int ret = recv(client_socket, recv_buffer, BUFFSIZE - 1, 0);
        // 返回值为0表示连接关闭，返回值小于0表示出错
        if (ret <= 0)
        {
            std::cout << "recv error" << std::endl;
            break;
        }
        printf("Recv: %s\n", recv_buffer);
    }
    close(client_socket);

    return 0;
}