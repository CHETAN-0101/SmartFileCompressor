#include "sfc/decompressor.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>
#include <cstring>
#include <zlib.h>

namespace sfc {

bool read_header(const std::string& input_path, SFCHeader& header) {
    std::ifstream infile(input_path, std::ios::binary);
    if (!infile) {
        return false;
    }
    infile.read(reinterpret_cast<char*>(&header), sizeof(SFCHeader));
    if (!infile || std::memcmp(header.magic, SFC_MAGIC, 4) != 0) {
        return false;
    }
    return true;
}

bool decompress_file(
    const std::string& input_path,
    const std::string& output_path,
    DecompressionStats* stats
) {
    auto start_time = std::chrono::high_resolution_clock::now();

    std::ifstream infile(input_path, std::ios::binary);
    if (!infile) {
        std::cerr << "decompress_file error: Cannot open input file: " << input_path << "\n";
        return false;
    }

    SFCHeader header{};
    infile.read(reinterpret_cast<char*>(&header), sizeof(SFCHeader));
    if (!infile || std::memcmp(header.magic, SFC_MAGIC, 4) != 0) {
        std::cerr << "decompress_file error: Invalid or corrupted SFC file header in " << input_path << "\n";
        return false;
    }

    std::ofstream outfile(output_path, std::ios::binary);
    if (!outfile) {
        std::cerr << "decompress_file error: Cannot create output file: " << output_path << "\n";
        return false;
    }

    z_stream strm;
    std::memset(&strm, 0, sizeof(strm));
    int ret = inflateInit2(&strm, MAX_WBITS + 16); // Match deflate format
    if (ret != Z_OK) {
        std::cerr << "decompress_file error: inflateInit2 failed with code " << ret << "\n";
        return false;
    }

    constexpr size_t BUFFER_SIZE = SFC_DEFAULT_CHUNK_SIZE;
    std::vector<Bytef> in_buffer(BUFFER_SIZE);
    std::vector<Bytef> out_buffer(BUFFER_SIZE);

    uLong computed_crc = crc32(0L, Z_NULL, 0);
    uint64_t total_written = 0;

    do {
        infile.read(reinterpret_cast<char*>(in_buffer.data()), BUFFER_SIZE);
        std::streamsize bytes_read = infile.gcount();
        if (bytes_read == 0) break;

        strm.next_in = in_buffer.data();
        strm.avail_in = static_cast<uInt>(bytes_read);

        do {
            strm.next_out = out_buffer.data();
            strm.avail_out = static_cast<uInt>(out_buffer.size());

            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                inflateEnd(&strm);
                std::cerr << "decompress_file error: Inflate error code " << ret << "\n";
                return false;
            }

            size_t produced = out_buffer.size() - strm.avail_out;
            if (produced > 0) {
                outfile.write(reinterpret_cast<const char*>(out_buffer.data()), produced);
                computed_crc = crc32(computed_crc, out_buffer.data(), static_cast<uInt>(produced));
                total_written += produced;
            }
        } while (strm.avail_out == 0);

    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    outfile.close();
    infile.close();

    auto end_time = std::chrono::high_resolution_clock::now();
    double duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    bool crc_valid = (static_cast<uint32_t>(computed_crc) == header.crc32);
    if (!crc_valid) {
        std::cerr << "decompress_file warning: CRC32 checksum mismatch! Expected: " 
                  << std::hex << header.crc32 << ", Computed: " << computed_crc << std::dec << "\n";
    }

    if (stats) {
        stats->header = header;
        stats->bytes_written = total_written;
        stats->computed_crc32 = static_cast<uint32_t>(computed_crc);
        stats->checksum_verified = crc_valid;
        stats->duration_ms = duration_ms;
    }

    return true;
}

} // namespace sfc
