#include "decompressor.h"
#include <iostream>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: sfc_viewer <.sfc.gz file>\n";
        return 1;
    }

    std::string input = argv[1];
    if (!fs::exists(input)) {
        std::cerr << "File not found: " << input << "\n";
        return 1;
    }

    std::string output = input.substr(0, input.find(".sfc.gz")); // restore original name
    if (fs::exists(output)) {
      std::cout << "Warning: Output file already exists. Overwriting...\n";
      fs::remove(output);
    }

    if (!decompress_file(input, output)) {
        std::cerr << "Failed to decompress\n";
        return 1;
    }

    std::cout << "Opening: " << output << "\n";
    std::string cmd = "xdg-open \"" + output + "\"";
    std::system(cmd.c_str());

    return 0;
}
