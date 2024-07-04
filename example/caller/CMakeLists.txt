# 设置需要编译的源文件列表
set(SRC_LIST callfriendservice.cc ../friend.pb.cc)
add_executable(consumer ${SRC_LIST})
# mprpc是自己创建的动态库
target_link_libraries(consumer mprpc protobuf)