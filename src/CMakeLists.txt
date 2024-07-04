aux_source_directory(. SRC_LIST)
# 生成一个动态库(而不是可执行文件)
# add_library(mprpc SHARED ${SRC_LIST})

# 因为muduo库是静态库，所以为避免错误也把mprpc设置为静态库
add_library(mprpc ${SRC_LIST})
# 先依赖muduo_net，再依赖muduo_base，库与库之间也有依赖关系
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)
#zookeeper_mt :多线程版本
