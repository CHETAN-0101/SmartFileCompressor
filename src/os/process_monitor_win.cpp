#include "sfc/process_monitor.h"
#include <iostream>
#include <filesystem>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <tlhelp32.h>
#endif

namespace sfc {
namespace os {

#if defined(_WIN32) || defined(_WIN64)

bool is_file_in_use(const std::string& target_filepath, std::vector<ProcessInfo>* locking_processes) {
    if (!std::filesystem::exists(target_filepath)) {
        return false;
    }

    // Attempt to open the file with zero sharing permissions to test for active handles/locks
    HANDLE hFile = CreateFileA(
        target_filepath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0, // Exclusive access test
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (err == ERROR_SHARING_VIOLATION || err == ERROR_LOCK_VIOLATION) {
            if (locking_processes) {
                // Populate process list if needed via Toolhelp
                ProcessInfo info;
                info.pid = 0;
                info.process_name = "Active Process Lock (Sharing Violation)";
                locking_processes->push_back(info);
            }
            return true;
        }
    } else {
        CloseHandle(hFile);
    }
    return false;
}

std::vector<ProcessInfo> get_running_processes() {
    std::vector<ProcessInfo> list;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return list;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnap, &pe)) {
        do {
            ProcessInfo info;
            info.pid = pe.th32ProcessID;
            info.process_name = pe.szExeFile;
            list.push_back(info);
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    return list;
}

#else

// Non-Windows fallbacks placed in process_monitor_nix.cpp
#endif

} // namespace os
} // namespace sfc
