#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

// 封装zk客户端的类
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();

    // zkclient启动连接zkserver
    void Start();

    // 在zkserver上根据指定的path创建znode节点，默认state为0，表示永久节点
    void Create(const char *path,const char* data,int datalen,int state=0);
    
    // 根据参数指定的znode节点路径，获取znode节点的值
    std::string GetData(const char* path);
private:
    // zk的客户端句柄(用来操控zkserver)
    zhandle_t *m_zhandle;
};