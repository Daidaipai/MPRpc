#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"

int main(int argc,char** argv){
    // 整个程序启动以后，想使用mprpc框架来享受rpc服务调用，一定要先调用框架的初始话函数(只初始化一次)
    MprpcApplication::Init(argc,argv);

    // 演示调用远程发布的rpc方法Login，先创建一个代理对象
    Daidaipai::UserServiceRpc_Stub stub(new MprpcChannel());// 基类构造传子类对象(多态)
    
    // // rpc方法的请求参数
    // Daidaipai::LoginRequest request;
    // request.set_name("ting zi");
    // request.set_pwd("123456");
    // // rpc方法的响应参数
    // Daidaipai::LoginResponse response;
    // // 发起rpc方法的调用，同步的rpc调用过程,最后都调用->MprpcChannel::CallMethod
    // stub.Login(nullptr,&request,&response,nullptr); // RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送
    
    // // 一次rpc调用完成，读调用的结果
    // if(response.result().errcode()==0){
    //     std::cout<<"rpc login response success: "<<response.success()<<std::endl;
    // }else{
    //     std::cout<<"rpc login response error: "<<response.result().errmsg()<<std::endl;
    // }

    // 演示调用远程发布的rpc方法Login
    Daidaipai::RegisterRequest req;
    req.set_id(8888);
    req.set_name("xin ge");
    req.set_pwd("123456");

    Daidaipai::RegisterResponse res;
    // 以同步的方式发起rpc请求调用，等待返回的结果
    stub.Register(nullptr,&req,&res,nullptr);

    // 一次rpc调用完成，读调用的结果
    if(res.result().errcode()==0){
        std::cout<<"rpc register response success: "<<res.success()<<std::endl;
    }else{
        std::cout<<"rpc register response error: "<<res.result().errmsg()<<std::endl;
    }

    return 0;
}