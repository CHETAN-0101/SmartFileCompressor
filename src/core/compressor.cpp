#include "sfc/compressor.h"
#include "sfc/mmap_file.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>
#include <cstring>
#include <zlib.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace sfc {

bool compress_file(
    const std::string& input_path,
    const std::string& output_path,
    const CompressionOptions& options,
    CompressionStats* stats
) {
    auto start_time = std::chrono::high_resolution_clock::now();

    MappedReadOnlyFile mmap_file;
    if (!mmap_file.open(input_path)) {
        std::cerr << "compress_file error: Failed to map input file: " << input_path << "\n";
        return false;
    }

    std::ofstream outfile(output_path, std::ios::binary);
    if (!outfile) {
        std::cerr << "compress_file error: Cannot open output file: " << output_path << "\n";
        return false;
    }

    // 1. Calculate CRC32 of original uncompressed payload
    const uint8_t* in_data = mmap_file.data();
    size_t in_size = mmap_file.size();
    uLong crc = crc32(0L, Z_NULL, 0);
    if (in_size > 0 && in_data) {
        crc = crc32(crc, (const Bytef*)in_data, (uInt)in_size);
    }

    // 2. Prepare SFC Header placeholder
    SFCHeader header{};
    std::memcpy(header.magic, SFC_MAGIC, 4);
    header.original_size = static_cast<uint64_t>(in_size);
    header.payload_size = 0; // Will update after compression
    header.crc32 = static_cast<uint32_t>(crc);
    header.algorithm_id = options.algorithm_id;
    header.flags = 0;

    // Get file mtime if available
    try {
        auto ftime = fs::last_write_time(input_path);
        auto s_time = std::chrono::duration_cast<std::chrono::seconds>(
            ftime.time_since_epoch()
        ).count();
        header.mtime = static_cast<int64_t>(s_time);
    } catch (...) {
        header.mtime = 0;
    }

    // Write header placeholder
    outfile.write(reinterpret_cast<const char*>(&header), sizeof(SFCHeader));

    // 3. Perform chunked zlib deflate streaming
    z_stream strm;
    std::memset(&strm, 0, sizeof(strm));
    int ret = deflateInit2(
        &strm,
        options.compression_level,
        Z_DEFLATED,
        MAX_WBITS + 16, // Gzip format wrapper or raw deflate
        8,
        Z_DEFAULT_STRATEGY
    );

    if (ret != Z_OK) {
        std::cerr << "compress_file error: deflateInit2 failed with code " << ret << "\n";
        return false;
    }

    size_t chunk_size = options.chunk_size > 0 ? options.chunk_size : SFC_DEFAULT_CHUNK_SIZE;
    std::vector<Bytef> out_buffer(chunk_size);
    uint64_t total_payload_size = 0;

    size_t bytes_remaining = in_size;
    const uint8_t* current_pos = in_data;

    do {
        size_t current_chunk = (bytes_remaining > chunk_size) ? chunk_size : bytes_remaining;
        strm.next_in = (Bytef*)current_pos;
        strm.avail_in = static_cast<uInt>(current_chunk);

        int flush = (current_chunk == bytes_remaining) ? Z_FINISH : Z_NO_FLUSH;
        current_pos += current_chunk;
        bytes_remaining -= current_chunk;

        do {
            strm.next_out = out_buffer.data();
            strm.avail_out = static_cast<uInt>(out_buffer.size());

            ret = deflate(&strm, flush);
            if (ret == Z_STREAM_ERROR) {
                deflateEnd(&strm);
                std::cerr << "compress_file error: deflate stream error\n";
                return false;
            }

            size_t produced = out_buffer.size() - strm.avail_out;
            if (produced > 0) {
                outfile.write(reinterpret_cast<const char*>(out_buffer.data()), produced);
                total_payload_size += produced;
            }
        } while (strm.avail_out == 0);

    } while (bytes_remaining > 0 || ret != Z_STREAM_END);

    deflateEnd(&strm);

    // 4. Update Header with true payload size
    header.payload_size = total_payload_size;
    outfile.seekp(0, std::ios::beg);
    outfile.write(reinterpret_cast<const char*>(&header), sizeof(SFCHeader));
    outfile.close();

    mmap_file.close();

    auto end_time = std::chrono::high_resolution_clock::now();
    double duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    if (stats) {
        stats->original_size = header.original_size;
        stats->compressed_size = sizeof(SFCHeader) + header.payload_size;
        stats->crc32 = header.crc32;
        stats->duration_ms = duration_ms;
        if (header.original_size > 0) {
            stats->ratio_percent = 100.0 * (1.0 - static_cast<double>(stats->compressed_size) / header.original_size);
        } else {
            stats->ratio_percent = 0.0;
        }
    }

    return true;
}

} // namespace sfc
