#include <gtest/gtest.h>
#include <iostream>
#include <zookeeper/zookeeper.h>
#include <semaphore.h>
#include <cstring>
#include <unistd.h>

using namespace std;

void watcher(zhandle_t* zh, int type, int state, const char* path, void* watcher_ctx) {
    if (type == ZOO_CREATED_EVENT) {
        cout << "节点 " << path << " 已被创建" << endl;
    } else if (type == ZOO_DELETED_EVENT) {
        cout << "节点 " << path << " 已被删除" << endl;
    } else if (type == ZOO_SESSION_EVENT) {
        if(state == ZOO_CONNECTED_STATE) {
            std::cout << "ZOO_CONNECTED_STATE" << std::endl;
            sem_t *sem = (sem_t*) zoo_get_context(zh);
            sem_post(sem);
        }
    }

    char buffer[1024];
    int buffer_len = sizeof(buffer);
    // 重新注册 watcher
    zoo_wget(zh, path, watcher, nullptr, buffer, &buffer_len, nullptr);
}


TEST(ZKTest, Test1) {
    const char* zk_hosts = "127.0.0.1:2181";
    zhandle_t* zh = zookeeper_init(zk_hosts, watcher, 20000, 0, nullptr, 0);
    if (!zh) {
        cerr << "无法连接到 ZooKeeper" << endl;
        return;
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(zh, &sem);
    sem_wait(&sem);

    char buffer[1024];
    int buffer_len = sizeof(buffer);
    const char* node_path = "/my_node";

    // 创建节点
    zoo_create(zh, node_path, "data", strlen("data"), &ZOO_OPEN_ACL_UNSAFE, 0, nullptr, 0);

    // 注册 watcher 监视节点
    zoo_wget(zh, node_path, watcher, nullptr, buffer, &buffer_len, nullptr);

    // 删除节点
    zoo_delete(zh, node_path, -1); // 使用 -1 表示无版本控制

    // 保持程序运行一段时间以接收事件
    cout << "等待事件..." << endl;
    sleep(5); // 让程序保持运行5秒

    // 再次创建节点
    zoo_create(zh, node_path, "new_data", strlen("new_data"), &ZOO_OPEN_ACL_UNSAFE, 0, nullptr, 0);

    // 保持程序运行一段时间以接收事件
    sleep(5);
    // 关闭 ZooKeeper 连接
    zookeeper_close(zh);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}