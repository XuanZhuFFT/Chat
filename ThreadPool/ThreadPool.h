#include<iostream>
#include<thread>
#include<mutex>
#include<string>
#include<condition_variable>
#include<queue>
#include<vector>
#include<functional>

#define MAX_THREADS 8 // 最大线程数
#define MAX_QUEUE_SIZE 100 // 最大任务队列大小

class ThreadPool{
    public:
        // 禁止拷贝构造和赋值
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        
        ThreadPool(int numThreads = MAX_THREADS);
        ~ThreadPool();

        template<class F, class... Args>
        void enqueue(F &&f,Args&&... args){
            std::function<void()> task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                tasks.emplace(move(task));
            }
            condition.notify_one();
            // 由于使用了 std::function<void()> 和 std::bind，它们的资源管理是由 C++ 标准库自动处理的，不需要手动释放task的资源
        }


    private:
        std::vector<std::thread> workers; // 线程池
        std::queue<std::function<void()>> tasks; // 任务队列
        std::mutex queue_mutex; // 互斥锁
        std::condition_variable condition; // 条件变量
        bool stop; // 停止标志
};