#pragma once

#include <string>
#include <cstdint>
#include <cstddef>
#include "sfc/common.h"

namespace sfc {

struct CompressionOptions {
    uint16_t algorithm_id = SFC_ALGO_ZLIB;
    int compression_level = 6;
    size_t chunk_size = SFC_DEFAULT_CHUNK_SIZE;
};

struct CompressionStats {
    uint64_t original_size = 0;
    uint64_t compressed_size = 0;
    uint32_t crc32 = 0;
    double ratio_percent = 0.0;
    double duration_ms = 0.0;
};

bool compress_file(
    const std::string& input_path,
    const std::string& output_path,
    const CompressionOptions& options = {},
    CompressionStats* stats = nullptr
);

} // namespace sfc
