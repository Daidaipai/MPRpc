#include "mprpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"

// 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
// 为保证框架的通用性，用基类接收派生类的对象
void RpcProvider::NotifyService(google::protobuf::Service * service)
{
    // 获取了服务对象的描述信息
    const google::protobuf::ServiceDescriptor* pserviceDesc=service->GetDescriptor();
    // 获取服务的名字
    std::string service_name=pserviceDesc->name();
    // 获取服务对象service的方法的数量
    int methodCnt=pserviceDesc->method_count();
    std::cout<<"service_name:"<<service_name<<std::endl;

    LOG_INFO("service_name:%s",service_name.c_str());

    ServiceInfo server_info;
    for(int i=0;i<methodCnt;i++){
        // 获取了服务对象指定下标的服务方法的描述 (抽象描述)
        const google::protobuf::MethodDescriptor* pmethodDesc=pserviceDesc->method(i);
        std::string method_name=pmethodDesc->name();
        server_info.m_methodMap.insert({method_name,pmethodDesc});
        std::cout<<"method_name:"<<method_name<<std::endl;

        LOG_INFO("method_name:%s",method_name.c_str());

    }
    server_info.m_service=service;
    m_serviceMap.insert({service_name,server_info});
}

// 启动rpc服务节点，开始提供rpc远程网络调用服务(muduo网络库)
void RpcProvider::Run()
{
    std::string ip=MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port=atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    muduo::net::InetAddress address(ip,port);

    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop,address,"RpcProvider");
    // 绑定连接回调和消息读写回调，分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this,std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    
    // 设置muduo库的线程数量
    server.setThreadNum(4);

    // 把当前rpc节点上发布的服务全部注册到zookeeper上，让rpc client可以从zookeeper上发现服务
    // session会话的timeout 30s，zkclient 网络I/O线程会在 1/3*timeout 时间发送ping心跳消息
    ZkClient zkCli;
    zkCli.Start();
    // 令service_name为永久性节点   method_name为临时性节点
    for(const auto &sp:m_serviceMap){
        std::string  service_path="/"+sp.first;
        zkCli.Create(service_path.c_str(),nullptr,0);// 第四个参数默认为零，表示永久性节点
        for(const auto&mp:sp.second.m_methodMap){
            std::string method_path=service_path+"/"+mp.first;
            char method_data[64]={0};
            sprintf(method_data,"%s:%d",ip.c_str(),port);
            // ZOO_EPHEMERAL表示znode是一个临时性节点
            zkCli.Create(method_path.c_str(),method_data,strlen(method_data),ZOO_EPHEMERAL);
        }
    }

    std::cout<<"RpcProvider start service at ip:"<<ip<<" port:"<<port<<std::endl;
    LOG_INFO("ip:%s",ip.c_str());

    // 启动网络服务
    server.start();
    m_eventLoop.loop();
}

// 新的连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if(!conn->connected()){
        conn->shutdown();// 断开连接
    }
}
/*
在框架内部，RpcProvider和RpcConsumer之间协商好通信用的protobuf数据类型
service_name method_name args    定义proto的message类型，进行数据头的序列化和反序列化
header_size(4个字节)+ header_str+ args_str
*/
// 已建立连接用户的读写事件回调，如果远程有一个rpc服务的调用请求，那么OnMessage方法就会响应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, 
                        muduo::net::Buffer *buffer, muduo::Timestamp time)
{
    // 网络上接收的远程rpc调用请求的字符流
    std::string recv_buf=buffer->retrieveAllAsString();

    // 从字符流中读取前4个字节的内容
    uint32_t header_size=0;
    recv_buf.copy((char*)&header_size,4,0);// 读取前4个字节赋值给header_size

    // 根据header_size读取数据头的原始字符流
    std::string rpc_header_str=recv_buf.substr(4,header_size);
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    mprpc::RpcHeader rpcHeader;
    if(rpcHeader.ParseFromString(rpc_header_str)){
        // 数据头反序列化成功
        service_name=rpcHeader.service_name();
        method_name=rpcHeader.method_name();
        args_size=rpcHeader.args_size();
    }else{
        // 数据头反序列化失败
        std::cout<<"rpc_header_str:"<<rpc_header_str<<" parse error!"<<std::endl;
        LOG_ERR("rpc_header_str:%s parse error!",rpc_header_str.c_str());
        return;
    }

    // 获取rpc方法参数的字符流数据
    std::string args_str=recv_buf.substr(4+header_size,args_size);

    // 打印调试信息
    std::cout<<"==============================="<<std::endl;
    std::cout<<"header_size: "<<header_size<<std::endl;
    std::cout<<"rpc_header_str: "<<rpc_header_str<<std::endl;
    std::cout<<"service_name: "<<service_name<<std::endl;
    std::cout<<"method_name: "<<method_name<<std::endl;
    std::cout<<"args_str: "<<args_str<<std::endl;
    std::cout<<"==============================="<<std::endl;

    // 获取service对象和method对象
    auto it=m_serviceMap.find(service_name);
    if(it==m_serviceMap.end()){
        std::cout<<service_name<<" is not exist!"<<std::endl;
        LOG_ERR("service_name:%s is not exist!",service_name.c_str());
        return;
    }
    auto itm=it->second.m_methodMap.find(method_name);
    if(itm==it->second.m_methodMap.end()){
        std::cout<<service_name<<":"<<method_name<<" is not exist!"<<std::endl;
        LOG_ERR("%s:%s is not exist!",service_name.c_str(),method_name.c_str());
        return;
    }
    google::protobuf::Service* service=it->second.m_service;// 获取service对象
    const google::protobuf::MethodDescriptor* method=itm->second;// 获取method对象的描述

    // 生成rpc方法调用的请求request和响应response
    google::protobuf::Message* request=service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str)){
        std::cout<<"args_str:"<<args_str<<" parse error!"<<std::endl;
        LOG_ERR("args_str:%s parse error!",args_str.c_str());
        return;
    }
    google::protobuf::Message* response=service->GetResponsePrototype(method).New();

    // 给下面的method方法的调用，绑定一个Closure的回调函数
    google::protobuf::Closure*done= // NewCallback函数会帮我们重写done->Run()方法(使用方便，避免了自己写)
        google::protobuf::NewCallback<RpcProvider,const muduo::net::TcpConnectionPtr &,
        google::protobuf::Message *>(this,&RpcProvider::SendRpcResponse,conn,response);

    // 在框架上根据远端的rpc请求，调用当前rpc节点上发布的方法
    // 相当于new UserService().Login(controller,request,response,done)
    service->CallMethod(method,nullptr,request,response,done);


}

// Closure的回调操作，用于序列化rpc响应和网络发送,用于done->Run()方法执行
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if(response->SerializePartialToString(&response_str)){
        // 序列化成功后，通过网络把rpc方法执行的结果发送给rpc的调用方
        conn->send(response_str);
    }else{
        std::cout<<"serialize response_str error!"<<std::endl;
        LOG_ERR("serialize response_str error!");
    }
    conn->shutdown();// 模拟http的短连接服务，由rpcprovider主动断开连接
}
