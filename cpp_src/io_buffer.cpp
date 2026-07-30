#include <vector>
#include <cstdint>
#include <cstddef>
#include <algorithm>

// Simple ring buffer for IO operations — used to exercise pointer math.
class IORingBuffer {
public:
    explicit IORingBuffer(size_t capacity) : buf_(capacity), head_(0), tail_(0), full_(false) {}
    size_t capacity() const { return buf_.size(); }
    size_t size() const { if (full_) return buf_.size(); if (tail_ >= head_) return tail_ - head_; return buf_.size() - (head_ - tail_); }
    bool push(const uint8_t* data, size_t n) {
        if (n > free_space()) return false;
        for (size_t i=0;i<n;++i) { buf_[tail_] = data[i]; tail_ = (tail_ + 1) % buf_.size(); if (tail_ == head_) full_ = true; }
        return true;
    }
    size_t pop(uint8_t* out, size_t n) {
        size_t avail = size(); size_t to = std::min(avail, n);
        for (size_t i=0;i<to;++i) { out[i] = buf_[head_]; head_ = (head_ + 1) % buf_.size(); full_ = false; }
        return to;
    }
private:
    std::vector<uint8_t> buf_;
    size_t head_, tail_;
    bool full_;
    size_t free_space() const { return capacity() - size(); }
};
