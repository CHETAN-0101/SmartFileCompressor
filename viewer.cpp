#include <iostream>
#include <filesystem>
#include "compressor.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: sfc <file_to_compress>\n";
        return 1;
    }

    std::string input = argv[1];
    if (!std::filesystem::exists(input)) {
        std::cerr << "Error: File does not exist.\n";
        return 1;
    }

    std::string output = input + ".sfc.gz";
    if (compress_file(input, output)) {
        std::cout << "Compressed to: " << output << "\n";
    } else {
        std::cerr << "Compression failed.\n";
    }

    return 0;
}
