#include "sfc/mmap_file.h"
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif

namespace sfc {

MappedReadOnlyFile::MappedReadOnlyFile() = default;

MappedReadOnlyFile::~MappedReadOnlyFile() {
    close();
}

bool MappedReadOnlyFile::open(const std::string& filepath) {
    close();

#if defined(_WIN32) || defined(_WIN64)
    HANDLE hFile = CreateFileA(
        filepath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "mmap_file error: Unable to open file " << filepath << "\n";
        return false;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        std::cerr << "mmap_file error: Unable to get file size for " << filepath << "\n";
        return false;
    }

    if (fileSize.QuadPart == 0) {
        // Empty file case
        CloseHandle(hFile);
        data_ = nullptr;
        size_ = 0;
        return true;
    }

    HANDLE hMap = CreateFileMappingA(
        hFile,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL
    );

    if (!hMap) {
        CloseHandle(hFile);
        std::cerr << "mmap_file error: CreateFileMapping failed for " << filepath << "\n";
        return false;
    }

    LPVOID pData = MapViewOfFile(
        hMap,
        FILE_MAP_READ,
        0,
        0,
        0
    );

    if (!pData) {
        CloseHandle(hMap);
        CloseHandle(hFile);
        std::cerr << "mmap_file error: MapViewOfFile failed for " << filepath << "\n";
        return false;
    }

    file_handle_ = hFile;
    map_handle_ = hMap;
    data_ = static_cast<const uint8_t*>(pData);
    size_ = static_cast<size_t>(fileSize.QuadPart);
    return true;

#else
    int fd = ::open(filepath.c_str(), O_RDONLY);
    if (fd < 0) {
        std::cerr << "mmap_file error: Unable to open file " << filepath << "\n";
        return false;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        ::close(fd);
        std::cerr << "mmap_file error: Unable to stat file " << filepath << "\n";
        return false;
    }

    if (st.st_size == 0) {
        ::close(fd);
        data_ = nullptr;
        size_ = 0;
        return true;
    }

    void* mapped = ::mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        ::close(fd);
        std::cerr << "mmap_file error: mmap failed for " << filepath << "\n";
        return false;
    }

    fd_ = fd;
    data_ = static_cast<const uint8_t*>(mapped);
    size_ = static_cast<size_t>(st.st_size);
    return true;
#endif
}

void MappedReadOnlyFile::close() {
#if defined(_WIN32) || defined(_WIN64)
    if (data_) {
        UnmapViewOfFile(const_cast<uint8_t*>(data_));
        data_ = nullptr;
    }
    if (map_handle_) {
        CloseHandle(static_cast<HANDLE>(map_handle_));
        map_handle_ = nullptr;
    }
    if (file_handle_) {
        CloseHandle(static_cast<HANDLE>(file_handle_));
        file_handle_ = nullptr;
    }
#else
    if (data_) {
        ::munmap(const_cast<uint8_t*>(data_), size_);
        data_ = nullptr;
    }
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
#endif
    size_ = 0;
}

} // namespace sfc
