#ifndef LOGGER_H
#define LOGGER_H

#include <cstdio>
#include <cstring>
#include <mutex>

#define LOG_BUFFSIZE 65536  

// 小型测试项目，采用互斥锁实现线程安全
// 如果并发量很大，考虑使用日志队列
class Logger
{
private:
    char buff[LOG_BUFFSIZE];
    int buffLen;
    FILE *fp;
    std::mutex mtx;  // 互斥锁，用于保护日志写入的线程安全

public:
    // 构造函数
    Logger();

    // 刷新缓存区到文件
    void Flush();

    // 记录日志
    void Log(const char *str, int len);

    // 析构函数
    ~Logger();
};

#endif // LOGGER_H
