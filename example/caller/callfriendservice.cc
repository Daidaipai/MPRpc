#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"


int main(int argc ,char**argv){
    MprpcApplication::Init(argc,argv);

    Daidaipai::FriendServiceRpc_Stub stub(new MprpcChannel());
    Daidaipai::GetFriendListRequest request;
    request.set_userid(8888);

    Daidaipai::GetFriendListResponse response;

    MprpcController controller;// 用来记录错误信息
    stub.GetFriendList(&controller,&request,&response,nullptr);

    if(controller.Failed()){
        std::cout<<controller.ErrorText()<<std::endl;
    }else{
        if(response.result().errcode()==0){
            std::cout<<"rpc GetFriendList response success!"<<std::endl;
            int size=response.friends_size();
            for(int i=0;i<size;i++){
                std::cout<< "index:"<<i+1<<"  name:"<<response.friends(i)<<std::endl;
            }
        }else{
            std::cout<<"rpc GetFriendList response error: "<<response.result().errmsg()<<std::endl;
        }
    }

    return 0;
}