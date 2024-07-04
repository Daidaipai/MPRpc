#include "zookeeperutil.h"
#include "mprpcapplication.h"

// 全局的watcher观察器，zkserver给zkclient的通知
void global_watcher(zhandle_t *zh, int type, 
        int state, const char *path,void *watcherCtx)
{
    if(type==ZOO_SESSION_EVENT){// 回调的消息类型是和会话相关的消息类型
        if(state==ZOO_CONNECTED_STATE){// zkclient和zkserver连接成功
            sem_t* sem=(sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}


ZkClient::ZkClient():m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    if(m_zhandle!=nullptr){
        // 关闭句柄，释放资源
        zookeeper_close(m_zhandle);
    }
}

// zkclient启动连接zkserver
void ZkClient::Start()
{
    std::string host=MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port=MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr=host+":"+port;

    /*
    zookeeper_mt :多线程版本
    zookeeper_init API提供了三个线程:
        API调用线程
        网络I/O线程 pthread_create  poll
        watcher回调线程 pthread_create
    */
    m_zhandle=zookeeper_init(connstr.c_str(),global_watcher,30000,nullptr,nullptr,0);
    if(m_zhandle==nullptr){
        std::cout<<"zookeeper_init error!"<<std::endl;
        exit(EXIT_FAILURE);
    }
    // m_zhandle不为空只是说明m_zhandle初始化成功，不代表连接成功(异步连接过程)
    // 需要在回调函数global_watcher中判断是否连接成功

    sem_t sem;
    sem_init(&sem,0,0);
    zoo_set_context(m_zhandle,&sem);

    // 阻塞在这等待global_watcher连接成功后的信号量通知
    sem_wait(&sem);
    std::cout<<"zookeeper_init success!"<<std::endl;
}

// 在zkserver上根据指定的path创建znode节点，默认state为0，表示永久节点
void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen=sizeof(path_buffer);
    int flag;
    // 先判断path表示的znode节点是否存在，如果存在就不再重复创建了
    flag=zoo_exists(m_zhandle,path,0,nullptr);
    if(flag==ZNONODE){// 表示path的znode节点不存在
        // 开始创建节点
        flag=zoo_create(m_zhandle,path,data,datalen,
            &ZOO_OPEN_ACL_UNSAFE,state,path_buffer,bufferlen);
        if(flag==ZOK){// 创建节点成功
            std::cout<<"znode create success! path:"<<path<<std::endl;
        }else{
            std::cout<<"flag:"<<flag<<std::endl;
            std::cout<<"znode create error! path:"<<path<<std::endl;
             exit(EXIT_FAILURE);
        }
    }
}

// 根据参数指定的znode节点路径，获取znode节点的值
std::string ZkClient::GetData(const char *path)
{
    char buffer[64]={0};
    int bufferlen=sizeof(buffer);
    int flag=zoo_get(m_zhandle,path,0,buffer,&bufferlen,nullptr);
    if(flag==ZOK){
        return buffer;
    }else{
        std::cout<<"get znode data error! path:"<<path<<std::endl;
        return "";
    }
}
