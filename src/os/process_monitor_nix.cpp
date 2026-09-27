#include "sfc/process_monitor.h"

#if !defined(_WIN32) && !defined(_WIN64)
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace sfc {
namespace os {

bool is_file_in_use(const std::string& target_filepath, std::vector<ProcessInfo>* locking_processes) {
    if (!fs::exists(target_filepath)) {
        return false;
    }

    int fd = open(target_filepath.c_str(), O_RDWR);
    if (fd < 0) {
        if (errno == EBUSY || errno == ETXTBSY || errno == EACCES) {
            return true;
        }
    } else {
        struct flock fl;
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;

        if (fcntl(fd, F_GETLK, &fl) == 0 && fl.l_type != F_UNLCK) {
            if (locking_processes) {
                ProcessInfo info;
                info.pid = fl.l_pid;
                info.process_name = "PID " + std::to_string(fl.l_pid);
                locking_processes->push_back(info);
            }
            close(fd);
            return true;
        }
        close(fd);
    }
    return false;
}

std::vector<ProcessInfo> get_running_processes() {
    std::vector<ProcessInfo> list;
    DIR* proc = opendir("/proc");
    if (!proc) return list;

    struct dirent* entry;
    while ((entry = readdir(proc)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            std::string name(entry->d_name);
            if (name.find_first_not_of("0123456789") == std::string::npos) {
                ProcessInfo info;
                info.pid = std::stoul(name);
                info.process_name = "PID " + name;
                list.push_back(info);
            }
        }
    }
    closedir(proc);
    return list;
}

} // namespace os
} // namespace sfc

#endif
