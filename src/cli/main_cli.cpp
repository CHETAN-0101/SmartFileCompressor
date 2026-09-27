#include <iostream>
#include <iomanip>
#include <filesystem>
#include "sfc/compressor.h"
#include "sfc/decompressor.h"
#include "sfc/process_monitor.h"
#include "sfc/ipc.h"

namespace fs = std::filesystem;

void print_banner() {
    std::cout << "========================================================\n";
    std::cout << "        SmartFileCompressor (SFC) v2.0 - OS Engine       \n";
    std::cout << "========================================================\n\n";
}

void print_usage(const char* prog) {
    print_banner();
    std::cout << "Usage:\n";
    std::cout << "  " << prog << " compress <file_path>           Compress file with SFC header & CRC32\n";
    std::cout << "  " << prog << " decompress <file.sfc>         Decompress file and verify CRC32\n";
    std::cout << "  " << prog << " inspect <file.sfc>            Display SFC binary header & statistics\n";
    std::cout << "  " << prog << " check-lock <file_path>        Inspect if OS process holds handle/lock\n";
    std::cout << "  " << prog << " daemon-status                 Query active background daemon via IPC\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string command = argv[1];

    if (command == "compress" && argc >= 3) {
        std::string input = argv[2];
        if (!fs::exists(input)) {
            std::cerr << "Error: File '" << input << "' does not exist.\n";
            return 1;
        }

        // Check if file is in use before compressing
        std::vector<sfc::os::ProcessInfo> locking_procs;
        if (sfc::os::is_file_in_use(input, &locking_procs)) {
            std::cerr << "Warning: File '" << input << "' is currently LOCKED or in use by another process!\n";
            for (const auto& proc : locking_procs) {
                std::cerr << "  - " << proc.process_name << " (PID: " << proc.pid << ")\n";
            }
            std::cerr << "Aborting compression to prevent data corruption.\n";
            return 1;
        }

        std::string output = input + ".sfc";
        sfc::CompressionOptions opts;
        sfc::CompressionStats stats;

        std::cout << "Compressing: " << input << " -> " << output << "...\n";
        if (sfc::compress_file(input, output, opts, &stats)) {
            std::cout << "\nCompression Successful!\n";
            std::cout << "----------------------------------------\n";
            std::cout << "Original Size   : " << stats.original_size << " bytes\n";
            std::cout << "Compressed Size : " << stats.compressed_size << " bytes\n";
            std::cout << "Space Saved     : " << std::fixed << std::setprecision(2) << stats.ratio_percent << "%\n";
            std::cout << "CRC32 Checksum  : 0x" << std::hex << stats.crc32 << std::dec << "\n";
            std::cout << "Time Elapsed    : " << stats.duration_ms << " ms\n";
            std::cout << "----------------------------------------\n";
            return 0;
        } else {
            std::cerr << "Error: Compression failed.\n";
            return 1;
        }
    }
    else if (command == "decompress" && argc >= 3) {
        std::string input = argv[2];
        if (!fs::exists(input)) {
            std::cerr << "Error: File '" << input << "' does not exist.\n";
            return 1;
        }

        std::string output = input;
        if (output.size() > 4 && output.substr(output.size() - 4) == ".sfc") {
            output = output.substr(0, output.size() - 4);
        } else {
            output = output + ".decompressed";
        }

        sfc::DecompressionStats stats;
        std::cout << "Decompressing: " << input << " -> " << output << "...\n";

        if (sfc::decompress_file(input, output, &stats)) {
            std::cout << "\nDecompression Successful!\n";
            std::cout << "----------------------------------------\n";
            std::cout << "Decompressed Size: " << stats.bytes_written << " bytes\n";
            std::cout << "CRC32 Verification: " << (stats.checksum_verified ? "PASSED (MATCHED)" : "FAILED (CORRUPTED)") << "\n";
            std::cout << "Time Elapsed     : " << stats.duration_ms << " ms\n";
            std::cout << "----------------------------------------\n";
            return stats.checksum_verified ? 0 : 1;
        } else {
            std::cerr << "Error: Decompression failed.\n";
            return 1;
        }
    }
    else if (command == "inspect" && argc >= 3) {
        std::string input = argv[2];
        sfc::SFCHeader header;
        if (sfc::read_header(input, header)) {
            std::cout << "\nSFC File Header Information: " << input << "\n";
            std::cout << "----------------------------------------\n";
            std::cout << "Magic Identifier  : " << std::string(header.magic, 4) << "\n";
            std::cout << "Original File Size: " << header.original_size << " bytes\n";
            std::cout << "Payload Size      : " << header.payload_size << " bytes\n";
            std::cout << "CRC32 Checksum    : 0x" << std::hex << header.crc32 << std::dec << "\n";
            std::cout << "Algorithm ID      : " << header.algorithm_id << " (Zlib/Deflate)\n";
            std::cout << "Modification Time : " << header.mtime << " (Epoch)\n";
            std::cout << "----------------------------------------\n";
            return 0;
        } else {
            std::cerr << "Error: Invalid SFC file format or unable to read header.\n";
            return 1;
        }
    }
    else if (command == "check-lock" && argc >= 3) {
        std::string target = argv[2];
        std::vector<sfc::os::ProcessInfo> procs;
        std::cout << "Inspecting OS process handles for: " << target << "...\n";
        bool locked = sfc::os::is_file_in_use(target, &procs);

        if (locked) {
            std::cout << "Status: LOCKED / IN USE BY SYSTEM PROCESSES!\n";
            for (const auto& p : procs) {
                std::cout << "  - PID: " << p.pid << " | Name: " << p.process_name << "\n";
            }
        } else {
            std::cout << "Status: FREE (No active process lock detected).\n";
        }
        return 0;
    }
    else if (command == "daemon-status") {
        std::cout << "Sending IPC status query to SFC Daemon...\n";
        std::string resp = sfc::ipc::IPCClient::send_command("STATUS");
        std::cout << "Daemon Response:\n" << resp << "\n";
        return 0;
    }
    else {
        print_usage(argv[0]);
        return 1;
    }
}
