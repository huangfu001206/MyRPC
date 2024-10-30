#pragma once
#include <string>
#include <semaphore.h>
#include <zookeeper/zookeeper.h>
class ZookeeperClient {
public:
    static ZookeeperClient& getInstance();
    ~ZookeeperClient();

    //连接zk_server
    void Run();

    //创建zk Znode
    void CreateZNode(const char* path, const char* data, int dataLen, int state = 0);

    //获取给定路径节点的信息
    std::string GetZNodeData(const char* path);
    
    //重新连接
    void ReConnect();

    //监听节点
    bool WatchNode(const std::string& path);

    //watcher函数
    static void WatchNodeHandler(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx);

private:
    ZookeeperClient();

    zhandle_t *zhandler;
    bool _is_connected;
};