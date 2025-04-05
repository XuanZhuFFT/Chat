#include "ThreadPool.h"

// 如果未指定numThreads，则使用宏MAX_THREADS
ThreadPool::ThreadPool(int numThreads):stop(false){
    // 如果没有传入 numThreads，则使用宏 MAX_THREADS 的值
    int threads_to_create;
    if(numThreads < 1){
        threads_to_create = MAX_THREADS;
    }else if(numThreads > 8){
        threads_to_create = MAX_THREADS;
        std::cout << "Up to 8 threads can be created" << std::endl;
    }else{
        threads_to_create = numThreads;
    }

    for(int i=0; i<threads_to_create; i++)
    {
        workers.emplace_back([this]{
            while(true)
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                // 线程阻塞等待，直到：stop = true 或 任务队列非空
                // lock：等待时释放锁，继续执行时重新加锁
                condition.wait(lock,[this]{return stop || !tasks.empty(); });

                if(stop && tasks.empty()){
                    return ;
                }

                // 从任务队列取出任务
                std::function<void()> task(std::move(tasks.front()));
                tasks.pop();

                // 解锁任务队列，开始执行任务
                lock.unlock();
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool(){
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        stop = true;
    }

    condition.notify_all();
    for (auto& worker : workers) {
        worker.join();
    }
}