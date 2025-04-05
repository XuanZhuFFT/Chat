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
#include <signal.h>
#include <thread>
#include <mutex>

#include "Logger/Logger.h"
#include "ThreadPool/ThreadPool.h"

#define BUFFSIZE 2048
#define DEFAULT_PORT 16555    // 指定端口为16555
#define MAXLINK 8
#define LOG_BUFFSIZE 65536

Logger logger; // 创建日志对象
ThreadPool threadpool;// 创建线程池

// 线程函数
void community(int client_socket){
    char buffer[BUFFSIZE];
    while(true){
        memset(buffer, 0, sizeof(buffer));

        int ret = recv(client_socket, buffer, sizeof(buffer), 0);
        if (ret <= 0) {
            printf("Client disconnected\n");
            break;
        }
        logger.Log("Recv: ", 6);
        logger.Log(buffer, strlen(buffer));

        send(client_socket, buffer, strlen(buffer), 0);
        logger.Log("Send: ", 6);
        logger.Log(buffer, strlen(buffer));
        logger.Log("\n", 1);
    }
    close(client_socket);
    printf("Client disconnected\n");
    logger.Log("Client disconnected\n", 21);
    logger.Flush();
}

int sockfd, connfd;

void setResponse(char *buffer)
{
    bzero(buffer, sizeof(buffer));
    // strcat函数：用于将src字符串追加到dest字符串的末尾
    // char *strcat (char *dest, const char *src);
    // dest：目标字符串，src：源字符串
    // strcat函数会将src字符串追加到dest字符串的末尾，并返回dest字符串的指针
    strcat(buffer, "HTTP/1.1 200 OK\r\n");
    strcat(buffer, "Connection: close\r\n");
    strcat(buffer, "\r\n");
    strcat(buffer, "Hello\n");
}

void stopServerRunning(int p)
{
    close(sockfd);
    printf("Close Server\n");
    exit(0);
}

int main(){
    // 服务器地址结构体
    struct sockaddr_in servaddr;

    // 用于收发数据的缓冲区
    char buffer[BUFFSIZE];
    // std::string buffer;
    // int send(int sockfd, const void *buf, size_t len, int flags);
    // send和recv函数不支持直接使用std::string类型

    //socket(int domain, int type, int protocol)
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    /*
      socket函数用于创建一个套接字，返回一个文件描述符（sockfd），用于后续的网络通信。
      AF_INET: IPv4协议 —— 指定通信域（或协议族），即通信双方的地址类型。
      SOCK_STREAM: TCP协议 —— 指定套接字的类型，决定了套接字的通信语义。
      0: 默认协议 —— 指定使用的协议。通常设置为 0，让系统自动选择默认协议。
    */

    if (sockfd == -1) {
        printf("Create socket error(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    /*
      socket函数返回 -1 表示函数调用失败
      errno 和 strerror(errno) 用于获取错误码和错误信息
    */

    bzero(&servaddr, sizeof(servaddr));// 清空servaddr的内容，初始化

    // 设置服务器地址结构体
    // 指定地址族为IPv4
    servaddr.sin_family = AF_INET;
    // 指定接收的IP地址，INADDR_ANY表示接收任意IP地址（具体来说，本机有多个可用的ip端口，指定接收哪个端口收到的请求）
    // htonl函数将主机字节序转换为网络字节序
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // 指定端口号，htons函数将主机字节序转换为网络字节序
    servaddr.sin_port = htons(DEFAULT_PORT);

    // bind函数（系统调用）：用于将套接字绑定到一个本地地址和端口。
    // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    if (-1 == bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)))
    {
        printf("Bind error(%d): %s\n", errno, strerror(errno));
        return -1;
    }


    // int listen(int sockfd, int backlog)
    // backlog：监听队列的最大长度；
    // 监听队列是指客户端向当前服务器发起了请求但未被accept的队列
    // 若队列已满，新的请求将被拒绝
    if (-1 == listen(sockfd, MAXLINK))
    {
        printf("Listen error(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    printf("Listening...\n");

    // void (*signal(int sig, void (*handler)(int)))(int);
    // 函数将信号sig与处理函数handler关联起来
    // 当接收到信号时，系统会调用handler函数来处理该信号
    // SIGINT：中断信号，通常由Ctrl+C触发
    signal(SIGINT, stopServerRunning);

    // 等待客户连接，服务器的具体处理逻辑
    while (true)
    {
        // int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
        // sockfd：监听套接字的文件描述符
        // addr：指向一个sockaddr结构体的指针，用于存储客户端的地址信息
        // addrlen：指向一个socklen_t类型的变量的指针，用于存储地址信息的长度
        int client_socket = accept(sockfd, NULL, NULL);
        // 服务器端存在两种套接字socket：
        // 1、监听套接字：用于监听客户端的连接请求，生命周期通常贯穿整个服务器程序的运行时间
        // 2、连接套接字：用于与客户端进行数据通信，每有一个连接（一个accept），产生一个连接套接字
        if (-1 == client_socket)
        {
            printf("Accept error(%d): %s\n", errno, strerror(errno));
            return -1;
        }
        
        threadpool.enqueue(community,client_socket);
    }

    return 0;

}