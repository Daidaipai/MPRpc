#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include <mprpcprovider.h>
using namespace std;
//using namespace Daidaipai;
/*
UserService原来是一个本地服务，提供了两个同一进程内的本地方法，Login和GetFriendLists
*/

class UserService:public Daidaipai::UserServiceRpc
{
public:
    bool Login(string name,string pwd)
    {
        cout<<"doing local service Login"<<endl;
        cout<<"name:"<<name<<" pwd:"<<pwd<<endl;
        return true;
    }

    bool Register(uint32_t id,std::string name,std::string pwd){
        cout<<"doing local service Register"<<endl;
        cout<<"id:"<<id<<" name:"<<name<<" pwd:"<<pwd<<endl;
        return true;
    }
    /*
    重写基类UserServiceRpc的虚函数，下面这些方法都是框架直接调用的
    1. caller ==> Login(LoginRequest) ==> muduo ==>callee
    2. callee ==> Login(LoginRequest) ==> 交到下面重写的Login方法上了
    */
    void Login(::google::protobuf::RpcController* controller,
                       const ::Daidaipai::LoginRequest* request,
                       ::Daidaipai::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
        // 框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        // 取出相应参数
        string name=request->name();
        string pwd=request->pwd();

        // 做本地业务
        bool login_result=Login(name,pwd);

        // 把响应结果写入框架
        Daidaipai::ResultCode*code= response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        // 执行回调操作，其中响应对象数据的序列化和网络发送(都是由框架来完成的)
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                       const ::Daidaipai::RegisterRequest* request,
                       ::Daidaipai::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t id=request->id();
        std::string name=request->name();
        std::string pwd=request->pwd();

        bool register_result=Register(id,name,pwd);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(register_result);

        done->Run();
    }
};

int main(int argc,char**argv)
{
    // 调用框架的初始化操作 
    MprpcApplication::Init(argc,argv);

    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    // 启动一个rpc服务发布节点，Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}