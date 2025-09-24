#include "compressor.h"
#include "vector"
#include <fstream>
#include <zlib.h>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

bool compress_file(const std::string& input_path, const std::string& output_path) {
    std::ifstream infile(input_path, std::ios::binary);
    if (!infile) {
        std::cerr << "Error: Cannot open input file.\n";
        return false;
    }

    std::ofstream outfile(output_path, std::ios::binary);
    if (!outfile) {
        std::cerr << "Error: Cannot open output file.\n";
        return false;
    }

    std::vector<char> in_data((std::istreambuf_iterator<char>(infile)), {});
    uLong src_size = in_data.size();
    uLong dest_size = compressBound(src_size);
    std::vector<Bytef> out_data(dest_size);

    int res = compress(out_data.data(), &dest_size, (const Bytef*)in_data.data(), src_size);
    if (res != Z_OK) {
        std::cerr << "Error: Compression failed with code " << res << "\n";
        return false;
    }

    outfile.write((char*)out_data.data(), dest_size);

    std::cout << "Original Size: " << src_size << " bytes\n";
    std::cout << "Compressed Size: " << dest_size << " bytes\n";

    return true;
}
