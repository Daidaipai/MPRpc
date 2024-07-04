#pragma once

#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"

// mprpc框架的基础类(单例类)，负责框架的初始化操作
class MprpcApplication
{
public:
    static void Init(int argc,char** argv);
    static MprpcApplication& GetInstance();
    MprpcConfig& GetConfig();// 此处不需要静态方法，因为由单例对象来调用
private:
    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&)=delete;// 删除拷贝构造
    MprpcApplication(MprpcApplication&&)=delete;// 删除移动构造

    static MprpcConfig m_config;// 静态类成员对象，保证将配置文件信息初始化到通用map里
};