#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

// 异步读写日志的日志队列
template<typename T>
class LockQueue{
public:
    void Push(const T&data);
    T Pop();
private:
    std::mutex m_mutex;
    std::condition_variable m_condvariable;
    std::queue<T> m_queue;
};

// 模板类型的只能在头文件中定义
// 多个worker线程都会向队列中写数据
template <typename T>
inline void LockQueue<T>::Push(const T &data)
{
    //m_mutex.lock();
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(data);
    m_condvariable.notify_one();
    //m_mutex.unlock();
}

// 一个线程读队列，写入日志文件中
template <typename T>
inline T LockQueue<T>::Pop()
{
    // condition_variable需要配合unique_lock进行wait操作
    std::unique_lock<std::mutex> lock(m_mutex);
    while(m_queue.empty()){
        // 日志队列为空，线程进入wait状态
        m_condvariable.wait(lock);
    }
    T data=m_queue.front();
    m_queue.pop();
    return data;
}
