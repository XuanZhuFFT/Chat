#include "Logger.h"

Logger::Logger()
{
    bzero(buff, sizeof(buff)); // 清空缓冲区
    buffLen = 0;
    fp = fopen("TrainServer.log", "a");  // 以追加模式打开日志文件
    if (!fp) {
        perror("Failed to open log file");
    }
}

void Logger::Flush()
{
    std::lock_guard<std::mutex> lock(mtx);  // 确保对日志写入操作进行加锁
    if (fp) {
        fputs(buff, fp);  // 将缓冲区内容写入文件
        fflush(fp);        // 刷新文件流缓冲
    }
    bzero(buff, sizeof(buff));  // 清空缓冲区
    buffLen = 0;  // 重置长度
}

void Logger::Log(const char *str, int len)
{
    constexpr int LOG_SAFETY_MARGIN = 10;
    if (buffLen + len >= LOG_BUFFSIZE - LOG_SAFETY_MARGIN)
    {
        Flush();  // 如果缓冲区空间不足，刷新缓冲区到文件
    }
    memcpy(buff + buffLen, str, len);  // 将新的日志内容复制到缓冲区
    buffLen += len;  // 更新当前缓冲区长度
}

Logger::~Logger()
{
    if (buffLen != 0)  // 如果缓冲区有内容，先刷新
    {
        Flush();
    }
    if (fp) {
        fclose(fp);  // 关闭文件
    }
}
