#include "mprpcchannel.h"
#include "rpcheader.pb.h"
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <error.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "mprpcapplication.h"
#include "mprpccontroller.h"
#include "zookeeperutil.h"

/*
序列化数据格式：header_size + service_name  method_name  args_size + args
*/
// 所有通过stub代理对象调用的rpc方法，都走到了这一步，统一做rpc方法调用数据的序列化和网络发送
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor * method, 
                                google::protobuf::RpcController * controller, 
                                const google::protobuf::Message * request, 
                                google::protobuf::Message * response,
                                google::protobuf::Closure * done)
{
    const google::protobuf::ServiceDescriptor*sd=method->service();
    std::string service_name=sd->name();
    std::string method_name=method->name();

    // 获取参数的序列化字符串长度 args_size
    uint32_t args_size=0;
    std::string args_str;
    if(request->SerializePartialToString(&args_str)){
        args_size=args_str.size();
    }else{
        controller->SetFailed("serialize request error!");
        return;
    }

    // 定义rpc请求的header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size =0;
    std::string rpc_header_str;
    if(rpcHeader.SerializePartialToString(&rpc_header_str)){
        header_size=rpc_header_str.size();
    }else{
        controller->SetFailed("serialize rpc header error!");
        return;
    }

    // 组织待发送的rpc请求的字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0,std::string((char*)&header_size,4));// 前四个字节用来存储header_size
    send_rpc_str+=rpc_header_str;
    send_rpc_str+=args_str;

    // 打印调试信息
    std::cout<<"==============================="<<std::endl;
    std::cout<<"header_size: "<<header_size<<std::endl;
    std::cout<<"rpc_header_str: "<<rpc_header_str<<std::endl;
    std::cout<<"service_name: "<<service_name<<std::endl;
    std::cout<<"method_name: "<<method_name<<std::endl;
    std::cout<<"args_str: "<<args_str<<std::endl;
    std::cout<<"==============================="<<std::endl;

    // 使用tcp方法，完成rpc方法的远程调用
    int clientfd=socket(AF_INET,SOCK_STREAM,0);
    if(clientfd==-1){
        char errtxt[512]={0};
        sprintf(errtxt,"creake socket error! error:%d",errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 读取配置文件的ip和port
    //std::string ip=MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    //uint16_t port=atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    
    // rpc调用方通过查询zk上该服务所在的host信息来获取ip和port，以调用service_name和method_name服务
    ZkClient zkCli;
    zkCli.Start();
    std::string method_path="/"+service_name+"/"+method_name;
    std::string host_data=zkCli.GetData(method_path.c_str());
    if(host_data==""){
        controller->SetFailed(method_path+" is not exist!");
        return;
    }
    int idx=host_data.find(":");
    if(idx==std::string::npos){
        controller->SetFailed(method_path+" is invalid!");
        return;
    }
    std::string ip=host_data.substr(0,idx);
    uint16_t port=atoi(host_data.substr(idx+1,host_data.size()-idx).c_str());

    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr=inet_addr(ip.c_str());

    // 连接rpc服务节点
    if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
    {
        close(clientfd);
        char errtxt[512]={0};
        sprintf(errtxt,"connect error! error:%d",errno);
        controller->SetFailed(errtxt);
        return;
    }
    // 发送rpc请求
    if(-1==send(clientfd,send_rpc_str.c_str(),send_rpc_str.size(),0)){
        close(clientfd);
        char errtxt[512]={0};
        sprintf(errtxt,"send error! error:%d",errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 接收rpc请求的响应值
    char recv_buf[1024]={0};
    int recv_size=0;
    if(-1==(recv_size=recv(clientfd,recv_buf,1024,0))){
        close(clientfd);
        char errtxt[512]={0};
        sprintf(errtxt,"recv error! error:%d",errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 反序列化rpc调用的响应数据
    //std::string response_str(recv_buf,0,recv_size);// 有bug，recv_buf中遇到\0，后面的数据就存不下来了，会导致反序列化失败
    //if(!response->ParseFromString(response_str)){
    if(!response->ParseFromArray(recv_buf,recv_size)){
        close(clientfd);
        char errtxt[1124]={0};
        sprintf(errtxt,"recv error! error:%s",recv_buf);
        controller->SetFailed(errtxt);
        return;
    }
    close(clientfd);
}