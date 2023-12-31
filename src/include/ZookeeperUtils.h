#pragma once
#include <string>
#include <semaphore.h>
#include <zookeeper/zookeeper.h>
class ZookeeperClient {
public:
    ZookeeperClient();
    ~ZookeeperClient();

    //连接zk_server
    void Run();

    //创建zk Znode
    void CreateZNode(const char* path, const char* data, int dataLen, int state = 0);

    //获取给定路径节点的信息
    std::string GetZNodeData(const char* path);

private:
    zhandle_t *zhandler;
};