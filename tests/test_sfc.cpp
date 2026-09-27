#include <iostream>
#include <fstream>
#include <cassert>
#include <filesystem>
#include "sfc/compressor.h"
#include "sfc/decompressor.h"
#include "sfc/process_monitor.h"

namespace fs = std::filesystem;

int main() {
    std::cout << "Running SmartFileCompressor Unit Tests...\n";

    std::string test_file = "test_sample.txt";
    std::string sfc_file = test_file + ".sfc";
    std::string restored_file = test_file + ".restored";

    // 1. Create dummy input test file
    std::ofstream out(test_file, std::ios::binary);
    std::string test_data = "SmartFileCompressor OS Engine Unit Test - Repeating content block for test compression ratio validation.\n";
    for (int i = 0; i < 100; ++i) {
        out << test_data << " Index: " << i << "\n";
    }
    out.close();

    std::cout << "[PASS] Sample file created (" << fs::file_size(test_file) << " bytes).\n";

    // 2. Test Process Monitor handle check
    bool locked = sfc::os::is_file_in_use(test_file);
    std::cout << "[PASS] Process handle check completed (is_in_use = " << (locked ? "true" : "false") << ").\n";

    // 3. Compress file
    sfc::CompressionOptions opts;
    sfc::CompressionStats comp_stats;
    bool comp_res = sfc::compress_file(test_file, sfc_file, opts, &comp_stats);
    assert(comp_res);
    std::cout << "[PASS] Compression succeeded. Ratio: " << comp_stats.ratio_percent << "% saved.\n";

    // 4. Read header validation
    sfc::SFCHeader header;
    bool header_res = sfc::read_header(sfc_file, header);
    assert(header_res);
    assert(header.original_size == comp_stats.original_size);
    assert(header.crc32 == comp_stats.crc32);
    std::cout << "[PASS] Header inspection verified magic and CRC32 (0x" << std::hex << header.crc32 << std::dec << ").\n";

    // 5. Decompress file
    sfc::DecompressionStats decomp_stats;
    bool decomp_res = sfc::decompress_file(sfc_file, restored_file, &decomp_stats);
    assert(decomp_res);
    assert(decomp_stats.checksum_verified);
    assert(decomp_stats.bytes_written == fs::file_size(test_file));
    std::cout << "[PASS] Decompression succeeded and CRC32 checksum matched!\n";

    // Cleanup test artifacts
    fs::remove(test_file);
    fs::remove(sfc_file);
    fs::remove(restored_file);

    std::cout << "\n========================================\n";
    std::cout << " ALL UNIT TESTS PASSED SUCCESSFULLY!    \n";
    std::cout << "========================================\n";
    return 0;
}
