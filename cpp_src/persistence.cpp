#include "inmem_db.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <mutex>

namespace inmembase {

// Very small persistence layer using a write-ahead log (WAL) and snapshot.
// The WAL records simple textual commands; snapshot writes a compact binary
// representation. This is not production-grade but demonstrates IO and
// durability concepts for fuzzing and review.

class Persistence {
public:
    Persistence(const std::string& path): path_(path) {}
    bool append_wal(const std::string& entry) {
        std::unique_lock lk(mu_);
        std::ofstream os(path_+"/wal.log", std::ios::app | std::ios::binary);
        if (!os.good()) return false;
        uint32_t len = (uint32_t)entry.size(); os.write((char*)&len, sizeof(len)); os.write(entry.data(), entry.size()); os.close(); return true;
    }
    std::vector<std::string> read_wal() {
        std::vector<std::string> out; std::ifstream is(path_+"/wal.log", std::ios::binary);
        if (!is.good()) return out; while (true) { uint32_t len; if (!is.read((char*)&len, sizeof(len))) break; std::string buf; buf.resize(len); if (!is.read(&buf[0], len)) break; out.push_back(buf); }
        return out;
    }
    bool write_snapshot(const Database& db) {
        std::unique_lock lk(mu_);
        std::ofstream os(path_+"/snapshot.bin", std::ios::binary);
        if (!os.good()) return false;
        // naive: write number of tables and table data
        auto tables = db.list_tables(); uint32_t tcount = (uint32_t)tables.size(); os.write((char*)&tcount, sizeof(tcount));
        for (const auto& tname : tables) {
            std::string name = tname; uint32_t nlen = (uint32_t)name.size(); os.write((char*)&nlen, sizeof(nlen)); os.write(name.data(), name.size());
            auto tabOpt = db.get_table(tname); if (!tabOpt) { uint32_t zero=0; os.write((char*)&zero,sizeof(zero)); continue; }
            Table tab = *tabOpt; // copy
            // columns
            uint32_t colc = (uint32_t)tab.columns().size(); os.write((char*)&colc, sizeof(colc));
            for (const auto& c : tab.columns()) {
                uint32_t n = (uint32_t)c.name.size(); os.write((char*)&n,sizeof(n)); os.write(c.name.data(), c.name.size()); uint32_t t = (uint32_t)c.type; os.write((char*)&t,sizeof(t)); uint8_t nullable = c.nullable?1:0; os.write((char*)&nullable,1);
            }
            // rows
            uint32_t rc = (uint32_t)tab.row_count(); os.write((char*)&rc,sizeof(rc));
            auto rows = tab.select_all();
            for (const auto& r : rows) {
                uint32_t cellc = (uint32_t)r.values.size(); os.write((char*)&cellc,sizeof(cellc));
                for (const auto& v : r.values) {
                    uint8_t tag = 0; if (std::holds_alternative<std::nullptr_t>(v)) tag=0; else if (std::holds_alternative<bool>(v)) tag=1; else if (std::holds_alternative<long long>(v)) tag=2; else if (std::holds_alternative<double>(v)) tag=3; else if (std::holds_alternative<std::string>(v)) tag=4;
                    os.write((char*)&tag,1);
                    if (tag==1) { uint8_t b = std::get<bool>(v)?1:0; os.write((char*)&b,1); }
                    else if (tag==2) { long long x = std::get<long long>(v); os.write((char*)&x,sizeof(x)); }
                    else if (tag==3) { double d = std::get<double>(v); os.write((char*)&d,sizeof(d)); }
                    else if (tag==4) { const std::string& s = std::get<std::string>(v); uint32_t ln=(uint32_t)s.size(); os.write((char*)&ln,sizeof(ln)); os.write(s.data(), s.size()); }
                }
            }
        }
        os.close(); return true;
    }
private:
    std::string path_;
    std::mutex mu_;
};

} // namespace inmembase
