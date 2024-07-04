#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
using namespace std;

MprpcConfig MprpcApplication::m_config;

void ShowArgHelp(){
    cout<<"format: command -i <configfile>"<<endl;
}

// 初始化，只需要初始化一次来读取配置文件，所以无需考虑map的线程安全
void MprpcApplication::Init(int argc, char ** argv)
{
    if(argc<3){
        ShowArgHelp();
        exit(EXIT_FAILURE);
    }
    
    int c=0;
    string config_file;
    // "i:"来表示i后面必须要跟着参数
    while((c=getopt(argc,argv,"i:"))!=-1){// 用于从命令行中筛选参数，如(provider -i config.conf)
        switch (c)
        {
        case 'i':
            config_file=optarg;// 返回i表示找到选项，并将其放入optarg中，然后赋值给config_file
            break;
        case '?':// 遇到了不认识的选项字符
            ShowArgHelp();
            exit(EXIT_FAILURE);
        case ':':// -i后面没有跟着选项
            ShowArgHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }
    // 开始加载配置文件了：rpcserver_ip=  rpcserver_port=  zookeeper_ip=  zookeeper_port=
    m_config.LoadConfigFile(config_file.c_str());

    // cout<<"rpcserverip:"<<m_config.Load("rpcserverip")<<endl;
    // cout<<"rpcserverport:"<<m_config.Load("rpcserverport")<<endl;
    // cout<<"zookeeperip:"<<m_config.Load("zookeeperip")<<endl;
    // cout<<"zookeeperport:"<<m_config.Load("zookeeperport")<<endl;
}

// 获取单例对象
MprpcApplication &MprpcApplication::GetInstance()
{
    static MprpcApplication app;// 此处一定要加static，以确保获得的是同一个单例对象，并且返回值是引用类型，不要返回局部对象
    return app;
}

MprpcConfig &MprpcApplication::GetConfig()
{
    return m_config;
}
