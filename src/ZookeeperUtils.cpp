#include "ZookeeperUtils.h"
#include "MyRpcApplication.h"
ZookeeperClient::ZookeeperClient():zhandler(nullptr) {

}

ZookeeperClient::~ZookeeperClient() {
    if(zhandler != nullptr) {
        zookeeper_close(zhandler);
    }
}

void ZkWatcher(zhandle_t* zhandle, int type, int state, const char* path, void *watcherCtx) {
    if(type == ZOO_SESSION_EVENT) {
        if(state == ZOO_CONNECTED_STATE) {
            //即连接成功
            sem_t *sem = (sem_t*) zoo_get_context(zhandle);
            sem_post(sem);
        }
    }
}

void ZookeeperClient::Run() {
    //获取zk server的ip和端口号
    std::string zk_ip = MyRpcApplication::getInstance().getFileInfo()["zookeeper_ip"];
    std::string zk_port = MyRpcApplication::getInstance().getFileInfo()["zookeeper_port"];
    std::string zk_addr = zk_ip+":"+zk_port;

    zhandler = zookeeper_init(zk_addr.c_str(), ZkWatcher, 50000, nullptr, nullptr, 0);
    if(zhandler == nullptr) {
        LOG_ERROR("zookeeper 初始化失败");
        exit(EXIT_FAILURE);
    }
    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(zhandler, &sem);
    sem_wait(&sem);
    LOG_INFO("zookeeper 初始化成功");
}

void ZookeeperClient::CreateZNode(const char *path, const char *data, int dataLen, int state) {
    char path_buffer[128];
    int bufferLen = sizeof(path_buffer);
    int flag;
    //首先判断节点是否存在
    flag = zoo_exists(zhandler, path, 0, nullptr);
    if(flag == ZNONODE) {
        //节点不存在，则进行创建
        flag = zoo_create(zhandler, path, data, dataLen, &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferLen);
        if(flag == ZOK) {
            LOG_INFO("ZNode 创建成功, 路径为：%s", path);
        } else {
            LOG_ERROR("节点创建失败， 路径为：%s", path);
            exit(EXIT_FAILURE);
        }
    }
}

std::string ZookeeperClient::GetZNodeData(const char *path) {
    char buffer[128];
    int bufferLen = sizeof(buffer);
    int flag = zoo_get(zhandler, path, 0, buffer, &bufferLen, nullptr);
    if(flag == ZOK) {
        LOG_INFO("获取ZNode 节点数据成功，路径为：%s", path);
        return buffer;
    }
    LOG_ERROR("获取节点数据失败，路径为：%s", path);
    return {};
}

