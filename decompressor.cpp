#include "decompressor.h"
#include <fstream>
#include <zlib.h>
#include <iostream>
#include <vector>

bool decompress_file(const std::string& input_path, const std::string& output_path) {
    std::ifstream infile(input_path, std::ios::binary);
    if (!infile) {
        std::cerr << "Error: Could not open input file: " << input_path << "\n";
        return false;
    }

    std::vector<char> in_data((std::istreambuf_iterator<char>(infile)), {});
    uLongf decompressed_size = in_data.size() * 10; // Estimate for buffer
    std::vector<Bytef> out_data(decompressed_size);

    int res = uncompress(out_data.data(), &decompressed_size, (const Bytef*)in_data.data(), in_data.size());
    if (res != Z_OK) {
        std::cerr << "Error: Inflate failed with code " << res << "\n";
        return false;
    }

    std::ofstream outfile(output_path, std::ios::binary);
    if (!outfile) {
        std::cerr << "Error: Could not create output file.\n";
        return false;
    }

    outfile.write((char*)out_data.data(), decompressed_size);

    std::cout << "Decompressed to: " << output_path << "\n";
    std::cout << "Decompressed Size: " << decompressed_size << " bytes\n";

    return true;
}
