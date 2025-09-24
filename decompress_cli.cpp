#include "decompressor.h"
#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: decompress_cli <file.sfc.gz>\n";
        return 1;
    }

    std::string input_path = argv[1];
    std::string output_path = input_path.substr(0, input_path.size() - 7);  // Remove ".sfc.gz"

    if (std::filesystem::exists(output_path)) {
        std::cerr << "Error: Output file '" << output_path << "' already exists\n";
        return 1;
    }

    if (decompress_file(input_path, output_path)) {
        std::cout << "Successfully decompressed: " << output_path << "\n";
        return 0;
    } else {
        std::cerr << "Decompression failed\n";
        return 1;
    }
}
