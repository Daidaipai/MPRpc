# 设置需要编译的源文件列表
set(SRC_LIST friendservice.cc ../friend.pb.cc)
add_executable(provider ${SRC_LIST})
# mprpc是自己创建的动态库
target_link_libraries(provider mprpc protobuf)
