#include <iostream>
#include "compressor.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_to_compress>\n";
        return 1;
    }
    std::string input = argv[1];
    std::string output = input + ".sfc.gz";
    if (compress_file(input, output)) {
        std::cout << "Compressed to: " << output << std::endl;
    } else {
        std::cerr << "Compression failed\n";
        return 1;
    }
    return 0;
}