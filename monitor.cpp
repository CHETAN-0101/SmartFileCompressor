//=== monitor.cpp ===
#include "monitor.h"
#include "compressor.h"
#include <sys/inotify.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <iostream>
#include <filesystem>
#include <linux/limits.h>

void watch_folder(const std::string& folder) {
    // Initialize inotify (non-blocking).
    int inotify_fd = inotify_init1(IN_NONBLOCK);
    if (inotify_fd < 0) {
        std::cerr << "Error: inotify_init1 failed: " << std::strerror(errno) << "\n";
        return;
    }
    // Add watch for files closed after write or moved in.
    int wd = inotify_add_watch(inotify_fd, folder.c_str(), IN_CLOSE_WRITE | IN_MOVED_TO);
    if (wd < 0) {
        std::cerr << "Error: inotify_add_watch failed: " << std::strerror(errno) << "\n";
        close(inotify_fd);
        return;
    }

    std::cout << "Monitoring folder: " << folder << " (press Ctrl+C to exit)\n";
    const size_t BUF_LEN = 1024 * (sizeof(struct inotify_event) + NAME_MAX + 1);
    char buffer[BUF_LEN];

    while (true) {
        int length = read(inotify_fd, buffer, BUF_LEN);
        if (length < 0) {
            if (errno == EAGAIN) { usleep(100000); continue; }
            std::cerr << "Error: inotify read failed: " << std::strerror(errno) << "\n";
            break;
        }
        // Process all events in buffer.
        for (int offset = 0; offset < length; ) {
            struct inotify_event *event = (struct inotify_event *)&buffer[offset];
            if (event->len > 0 && !(event->mask & IN_ISDIR)) {
                std::string filename(event->name);
                // Skip already-compressed files.
                if (filename.size() > 7 && filename.substr(filename.size()-7) == ".sfc.gz") {
                    // ignore
                } else {
                    std::string fullpath = folder + "/" + filename;
                    std::string outpath = fullpath + ".sfc.gz";
                    std::cout << "Compressing: " << filename << " -> " << filename << ".sfc.gz\n";
                    if (compress_file(fullpath, outpath)) {
                        if (std::remove(fullpath.c_str()) != 0) {
                            std::cerr << "Warning: failed to remove '" << fullpath << "'\n";
                        }
                    } else {
                        std::cerr << "Error: compression failed for '" << fullpath << "'\n";
                    }
                }
            }
            offset += sizeof(struct inotify_event) + event->len;
        }
    }
    inotify_rm_watch(inotify_fd, wd);
    close(inotify_fd);
}
