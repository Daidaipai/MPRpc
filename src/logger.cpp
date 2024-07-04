#include "logger.h"
#include <time.h>
#include <iostream>

// 获取单例对象
Logger &Logger::GetInstance()
{
    static Logger logger;
    return logger;
}

Logger::Logger(){
    // 启动专门的写日志线程
    std::thread writeLogTask([&](){
        while(1){
            // 获取当前的日期，然后获取队列信息，写入相应的日志文件当中
            time_t now=time(nullptr);
            tm* nowtm=localtime(&now);// tm是个结构体，其中存储了年月日等信息

            char file_name[128]={0};
            sprintf(file_name,"%d-%d-%d-log.txt",nowtm->tm_year+1900,nowtm->tm_mon+1,nowtm->tm_mday);

            FILE*pf=fopen(file_name,"a+");// a+以追加的模式写入数据，如果文件不存在则创建文件
            if(pf==nullptr){
                std::cout<<"logger file: "<<file_name<<" open error!"<<std::endl;
                exit(EXIT_FAILURE);
            }
            // 当队列为空时会阻塞在这里，直到队列中放入数据被唤醒
            std::string msg=m_lckQue.Pop();

            char time_buf[128]={0};
            sprintf(time_buf,"%d-%d-%d->[%s] ",
                nowtm->tm_hour,nowtm->tm_min,nowtm->tm_sec,(m_loglevel==INFO?"info":"error"));
            msg.insert(0,time_buf);// 在数据前插入时间信息
            msg+="\n";

            fputs(msg.c_str(),pf);// 将数据写入文件
            fclose(pf);
        }
    });
    // 设置分离线程，守护线程,否则会导致程序崩溃
    writeLogTask.detach();
}

// 设置日志级别
void Logger::SetLogLevel(LogLevel level)
{
    m_loglevel=level;
}

// 写日志，把日志信息写入lockqueue缓冲区中
void Logger::Log(std::string msg)
{
    m_lckQue.Push(msg);
}
