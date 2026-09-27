#pragma once

#include <string>
#include <cstdint>
#include "sfc/common.h"

namespace sfc {

struct DecompressionStats {
    SFCHeader header{};
    uint64_t bytes_written = 0;
    uint32_t computed_crc32 = 0;
    bool checksum_verified = false;
    double duration_ms = 0.0;
};

bool decompress_file(
    const std::string& input_path,
    const std::string& output_path,
    DecompressionStats* stats = nullptr
);

bool read_header(const std::string& input_path, SFCHeader& header);

} // namespace sfc
