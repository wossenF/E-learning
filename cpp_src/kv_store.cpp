#include <map>
#include <string>
#include <mutex>
#include <fstream>

// Tiny in-memory key-value store with optional simple file persistence.
class KVStore {
public:
    void put(const std::string& k, const std::string& v) {
        std::lock_guard<std::mutex> lk(m_);
        store_[k] = v;
    }
    bool get(const std::string& k, std::string& out) {
        std::lock_guard<std::mutex> lk(m_);
        auto it = store_.find(k);
        if (it == store_.end()) return false;
        out = it->second; return true;
    }
    void remove(const std::string& k) { std::lock_guard<std::mutex> lk(m_); store_.erase(k); }
    bool persist(const std::string& path) {
        std::lock_guard<std::mutex> lk(m_);
        std::ofstream ofs(path, std::ios::binary);
        if (!ofs.good()) return false;
        for (const auto& kv : store_) {
            uint32_t klen = (uint32_t)kv.first.size(); uint32_t vlen = (uint32_t)kv.second.size();
            ofs.write(reinterpret_cast<const char*>(&klen), sizeof(klen)); ofs.write(kv.first.data(), klen);
            ofs.write(reinterpret_cast<const char*>(&vlen), sizeof(vlen)); ofs.write(kv.second.data(), vlen);
        }
        return true;
    }
    bool load(const std::string& path) {
        std::lock_guard<std::mutex> lk(m_);
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs.good()) return false;
        store_.clear();
        while (ifs) {
            uint32_t klen=0; ifs.read(reinterpret_cast<char*>(&klen), sizeof(klen)); if (!ifs) break;
            std::string k(klen, '\0'); ifs.read(&k[0], klen);
            uint32_t vlen=0; ifs.read(reinterpret_cast<char*>(&vlen), sizeof(vlen));
            std::string v(vlen, '\0'); ifs.read(&v[0], vlen);
            store_[k]=v;
        }
        return true;
    }
private:
    std::map<std::string,std::string> store_;
    std::mutex m_;
};
