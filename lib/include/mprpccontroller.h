#pragma once

#include <google/protobuf/service.h>
#include <string>

// 用户可以根据MprpcController中的方法得知运行状态信息
class MprpcController:public google::protobuf::RpcController{
public:
    MprpcController();
    void Reset();
    bool Failed() const;
    std::string ErrorText() const;
    void SetFailed(const std::string& reason);

    // 目前未实现的功能
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);
private:
    bool m_failed;// rpc方法执行过程中的状态
    std::string m_errText;// rpc方法执行过程中的错误信息
};