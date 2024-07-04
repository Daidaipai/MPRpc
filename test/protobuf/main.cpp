#include "test.pb.h"
#include <iostream>
#include <string>
using namespace Daidaipai;
using namespace std;

int main1(){
    // LoginResponse rsp;
    // ResultCode *rc=rsp.mutable_result();
    // rc->set_errcode(1);
    // rc->set_errmsg("登陆处理失败了");

    GetFriendListsResponse rsp;
    ResultCode *rc=rsp.mutable_result();
    rc->set_errcode(0);

    User *user1= rsp.add_friend_list();
    user1->set_name("ting zi");
    user1->set_age(24);
    user1->set_sex(User::WOMAN);

    User *user2=rsp.add_friend_list();
    user2->set_name("xin ge");
    user2->set_age(24);
    user2->set_sex(User::MAN);

    cout<<rsp.friend_list_size()<<endl;

    return 0;
}

int main(){
    // 封装了login请求对象的数据
    LoginRequest req;
    req.set_name("ting zi");
    req.set_pwd("123456");

    // 对象数据序列化
    string send_str;
    // 传出参数，将序列化后的字节流存入里面
    if(req.SerializeToString(&send_str)){
        cout<<send_str<<endl;
    }

    // 从send_str反序列化一个login请求对象
    LoginRequest reqp;
    if(reqp.ParseFromString(send_str)){
        cout<<reqp.name()<<endl;
        cout<<reqp.pwd()<<endl;
    }
    return 0;
}

