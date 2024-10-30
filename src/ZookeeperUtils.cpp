#include "ZookeeperUtils.h"
#include "MyRpcApplication.h"
#include "DbLogUtils.h"
#include "LRUCache.h"
#include <sstream>
#include <string>

ZookeeperClient& ZookeeperClient::getInstance() {
    static ZookeeperClient client;
    return client;
}

ZookeeperClient::ZookeeperClient():zhandler(nullptr), _is_connected(false) {

}

ZookeeperClient::~ZookeeperClient() {
    if(zhandler != nullptr) {
        zookeeper_close(zhandler);
    }
}

void ZkWatcher(zhandle_t* zhandle, int type, int state, const char* path, void *watcherCtx) {
    // std::cout << &zhandle << std::endl;
    std::cout << type << " " << state << " " << ZOO_CONNECTED_STATE << " " <<ZOO_EXPIRED_SESSION_STATE << std::endl;
    std::cout << path << std::endl;
    if(type == ZOO_SESSION_EVENT) {
        if(state == ZOO_CONNECTED_STATE) {
            std::cout << "ZOO_CONNECTED_STATE" << std::endl;
            sem_t *sem = (sem_t*) zoo_get_context(zhandle);
            sem_post(sem);
        } else if(state == ZOO_EXPIRED_SESSION_STATE) {
            std::cout << "ZOO_EXPIRED_SESSION_STATE" << std::endl;
            ZookeeperClient::getInstance().ReConnect();
        } else if(state == ZOO_CONNECTING_STATE) {
            std::cout << "Connecting to Zookeeper..." << std::endl;
        }
    } 
}

void ZookeeperClient::ReConnect() {
    // std::cout << "***** ReConnect ******" << std::endl;
    if (_is_connected) {
        zookeeper_close(zhandler);
        _is_connected = false;
    }
    Run(); // 重新连接
}

void ZookeeperClient::WatchNodeHandler(zhandle_t* zh, int type, int state, const char* path, void* watcher_ctx) {
    // 重新获取节点数据
    char buffer[1024];
    int buffer_len = sizeof(buffer);

    // 处理节点事件
    if (type == ZOO_CREATED_EVENT) {
        std::cout << "Node created: " << path << std::endl;
    } else if (type == ZOO_DELETED_EVENT) {
        std::cout << "Node deleted: " << path << std::endl;
    } else if (type == ZOO_CHANGED_EVENT) {
        std::cout << "Node data changed: " << path << std::endl;
    } else if (type == ZOO_CHILD_EVENT) {
        std::cout << "Children changed in node: " << path << std::endl;
    } else {
        std::cout << "Unhandled event type: " << type << std::endl;
    }

    auto& cache = LRUCache<std::string, std::string>::getInstance();
    std::string node_path = std::string(path);

    if (type == ZOO_CHANGED_EVENT) {
        if (zoo_get(zh, path, 0, buffer, &buffer_len, nullptr) == ZOK) {
            buffer[buffer_len] = '\0'; // 确保字符串结束
        } //TODO : Exception Cache
        std::string new_ip_port = std::string(buffer);
        cache.put(node_path, new_ip_port);
        std::cout << "new config : " << new_ip_port << std::endl;
    } else if (type == ZOO_DELETED_EVENT) {
        cache.remove(path);
        std::cout << "Node deleted: " << path << std::endl;
    } else if (type == ZOO_CREATED_EVENT) {
        std::cout << "Node created: " << path << std::endl;
    }

    // 重新设置 watcher
    zoo_wget(zh, path, &ZookeeperClient::WatchNodeHandler, nullptr, buffer, &buffer_len, nullptr);
}

bool ZookeeperClient::WatchNode(const std::string& path) {
    char buffer[1024];
    int buffer_len = sizeof(buffer);
    if (zoo_wget(zhandler, path.c_str(), &ZookeeperClient::WatchNodeHandler, nullptr, buffer, &buffer_len, nullptr) == ZOK) {
        buffer[buffer_len] = '\0'; // 确保字符串结束
        LOG_INFO("Current data for path : %s", buffer);
        return true;
    } else {
        LOG_ERROR("Failed to set watcher on node: %s", path.c_str());
        return false;
    }
}

void ZookeeperClient::Run() {
    //避免重复连接
    if(_is_connected) {
        return;
    }
    //获取zk server的ip和端口号
    std::string zk_ip = MyRpcApplication::getInstance().getFileInfo()["zookeeper_ip"];
    std::string zk_port = MyRpcApplication::getInstance().getFileInfo()["zookeeper_port"];
    std::string zk_addr = zk_ip+":"+zk_port;

    zhandler = zookeeper_init(zk_addr.c_str(), ZkWatcher, 5000, nullptr, nullptr, 0);
    if(zhandler == nullptr) {
        LOG_ERROR("zookeeper 初始化失败");
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(zhandler, &sem);
    sem_wait(&sem);
    _is_connected = true;
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

