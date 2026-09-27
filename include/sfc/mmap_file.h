#pragma once

#include <string>
#include <cstddef>
#include <cstdint>

namespace sfc {

class MappedReadOnlyFile {
public:
    MappedReadOnlyFile();
    ~MappedReadOnlyFile();

    // Prevent copying
    MappedReadOnlyFile(const MappedReadOnlyFile&) = delete;
    MappedReadOnlyFile& operator=(const MappedReadOnlyFile&) = delete;

    bool open(const std::string& filepath);
    void close();

    const uint8_t* data() const { return data_; }
    size_t size() const { return size_; }
    bool is_open() const { return data_ != nullptr; }

private:
    const uint8_t* data_ = nullptr;
    size_t size_ = 0;

#if defined(_WIN32) || defined(_WIN64)
    void* file_handle_ = nullptr;
    void* map_handle_ = nullptr;
#else
    int fd_ = -1;
#endif
};

} // namespace sfc
