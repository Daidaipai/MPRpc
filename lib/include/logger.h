#pragma once
#include "lockqueue.h"
#include <string>

// 定义宏
// Logger logger=Logger::GetInstance();\  因为拷贝构造函数已经delete，所以此处只能用引用类型

#define LOG_INFO(logmsgformat, ...)\
    do  \
    {   \
        Logger& logger=Logger::GetInstance();\
        logger.SetLogLevel(INFO);\
        char cr[1024]={0};\
        snprintf(cr,1024,logmsgformat,##__VA_ARGS__);\
        logger.Log(cr);\
    }while(0);

#define LOG_ERR(logmsgformat, ...)\
    do  \
    {   \
        Logger& logger=Logger::GetInstance();\
        logger.SetLogLevel(ERROR);\
        char cr[1024]={0};\
        snprintf(cr,1024,logmsgformat,##__VA_ARGS__);\
        logger.Log(cr);\
    }while(0);

enum LogLevel
{
    INFO,// 普通信息
    ERROR// 错误信息
};


// Mprpc框架提供的日志系统
class Logger{
public:
    // 获取日志的单例，静态的、返回引用的
    static Logger& GetInstance();
    // 设置日志级别
    void SetLogLevel(LogLevel level);
    // 写日志
    void Log(std::string msg);
private:
    int m_loglevel;// 记录日志级别
    LockQueue<std::string> m_lckQue;// 日志缓冲队列

    // 设置成单例模式
    Logger();
    Logger(const Logger&)=delete;// 删除拷贝构造
    Logger(Logger&&)=delete;// 删除移动构造
};

