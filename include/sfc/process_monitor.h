#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace sfc {
namespace os {

struct ProcessInfo {
    uint32_t pid = 0;
    std::string process_name;
};

// Returns true if target file is currently opened/locked by any active system process
bool is_file_in_use(const std::string& target_filepath, std::vector<ProcessInfo>* locking_processes = nullptr);

// Retrieves list of running processes on the system
std::vector<ProcessInfo> get_running_processes();

} // namespace os
} // namespace sfc
