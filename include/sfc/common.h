#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

namespace sfc {

#pragma pack(push, 1)
struct SFCHeader {
    char magic[4];           // Magic bytes: "SFC1"
    uint64_t original_size;  // Original uncompressed size (bytes)
    uint64_t payload_size;   // Compressed payload size (bytes)
    uint32_t crc32;          // CRC32 checksum of uncompressed content
    uint16_t algorithm_id;   // 0 = Zlib/Deflate, 1 = LZ4, 2 = Zstd
    uint16_t flags;          // Reserved for encryption/flags
    int64_t  mtime;          // Original modification timestamp
};
#pragma pack(pop)

constexpr char SFC_MAGIC[4] = {'S', 'F', 'C', '1'};
constexpr uint16_t SFC_ALGO_ZLIB = 0;
constexpr size_t SFC_DEFAULT_CHUNK_SIZE = 64 * 1024; // 64KB chunks

} // namespace sfc
