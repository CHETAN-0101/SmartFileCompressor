#include <iostream>
#include <thread>
#include <chrono>
#include "sfc/thread_pool.h"
#include "sfc/scheduler.h"
#include "sfc/ipc.h"

int main() {
    std::cout << "Starting SmartFileCompressor Daemon (sfcd)...\n";
    
    // Set process to low background I/O priority
    sfc::os::set_process_low_io_priority();
    std::cout << "Configured Process I/O Priority: Low/Background Mode\n";

    // Initialize OS-aware ThreadPool with hardware core count
    sfc::ThreadPool pool(std::thread::hardware_concurrency(), sfc::PriorityLevel::Low);
    std::cout << "Initialized Worker ThreadPool with " 
              << std::thread::hardware_concurrency() << " Low-Priority Threads\n";

    // Start IPC Server
    sfc::ipc::IPCServer server;
    bool started = server.start([&pool](const std::string& req) -> std::string {
        if (req == "STATUS") {
            std::string status = "=== SFC Daemon Status ===\n";
            status += "Active Worker Threads: " + std::to_string(pool.active_workers()) + "\n";
            status += "Pending Queue Size:    " + std::to_string(pool.queue_size()) + "\n";
            status += "Paused State:          " + std::string(pool.is_paused() ? "YES" : "NO") + "\n";
            return status;
        } else if (req == "PAUSE") {
            pool.pause();
            return "Daemon ThreadPool Paused.";
        } else if (req == "RESUME") {
            pool.resume();
            return "Daemon ThreadPool Resumed.";
        }
        return "ERROR: Unknown command: " + req;
    });

    if (started) {
        std::cout << "IPC Server running successfully. Listening for commands...\n";
        std::cout << "Press Ctrl+C to terminate daemon.\n";
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } else {
        std::cerr << "Failed to start IPC Server.\n";
        return 1;
    }

    return 0;
}
