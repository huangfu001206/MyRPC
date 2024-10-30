#ifndef LRU_CACHE_H
#define LRU_CACHE_H
#include <iostream>
#include <unordered_map>
#include <list>
#include <shared_mutex>
#include <memory>

template <typename K, typename V>
class LRUCache {
public:
    static LRUCache& getInstance() {
        static LRUCache _cache;
        return _cache;
    }

    void set_capacity(size_t capacity) {_capacity = capacity;}

    std::shared_ptr<V> get(const K& key) {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        auto it = _cache_map.find(key);
        if (it == _cache_map.end()) {
            return nullptr; 
        }

        _access_list.splice(_access_list.begin(), _access_list, it->second.second);
        return std::make_shared<V>(it->second.first); // Return a copy of the value
    }

    void put(const K& key, const V& value) {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        auto it = _cache_map.find(key);
        if (it != _cache_map.end()) {
            // Key already exists, update value and move to front
            it->second.first = value;
            _access_list.splice(_access_list.begin(), _access_list, it->second.second);
        } else {
            // Key does not exist, insert new value
            if (_cache_map.size() >= _capacity) {
                // Evict the least recently used item
                const K& lru_key = _access_list.back();
                _access_list.pop_back();
                _cache_map.erase(lru_key);
            }
            _access_list.push_front(key);
            _cache_map[key] = {value, _access_list.begin()};
        }
    }

    void remove(const K& key) {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        auto it = _cache_map.find(key);
        if (it != _cache_map.end()) {
            // Remove from access list and cache map
            _access_list.erase(it->second.second);
            _cache_map.erase(it);
            std::cout << "删除缓存: " << key << std::endl;
        }
    }

    // void printCache() {
    //     std::shared_lock<std::shared_mutex> lock(_mutex);
    //     for (const auto& key : _access_list) {
    //         std::cout << key << ": " << _cache_map[key].first << std::endl;
    //     }
    // }

private:
    size_t _capacity = 100;
    std::list<K> _access_list; // Store keys in order of access
    std::unordered_map<K, std::pair<V, typename std::list<K>::iterator>> _cache_map; // Key -> (Value, Iterator)
    mutable std::shared_mutex _mutex; // For thread safety

    LRUCache() = default;
};

#endif // LRU_CACHE_H