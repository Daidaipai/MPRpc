#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include <mprpcprovider.h>
#include <vector>
#include "logger.h"

class FriendService:public::Daidaipai::FriendServiceRpc{
public:
    std::vector<std::string> GetFriendList(uint32_t userid){
        std::cout<<"doing local service GetFriendList,userid:"<<userid<<std::endl;
        std::vector<std::string>vec;
        vec.push_back("ting zi");
        vec.push_back("xin ge");
        vec.push_back("sang sang");
        return vec;
    }

    void GetFriendList(::google::protobuf::RpcController* controller,
                       const ::Daidaipai::GetFriendListRequest* request,
                       ::Daidaipai::GetFriendListResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t userid=request->userid();

        std::vector<std::string> friendsList=GetFriendList(userid);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");        
        for(auto &name:friendsList){
            std::string* p=response->add_friends();
            *p=name;
        }

        done->Run();
    }

};

int main(int argc,char**argv){
    // 调用框架的初始化操作 
    MprpcApplication::Init(argc,argv);

    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点，Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();
    
    return 0;
}