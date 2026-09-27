#include "sfc/scheduler.h"
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#endif

namespace sfc {
namespace os {

void set_current_thread_priority(PriorityLevel level) {
#if defined(_WIN32) || defined(_WIN64)
    HANDLE hThread = GetCurrentThread();
    switch (level) {
        case PriorityLevel::Normal:
            SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);
            break;
        case PriorityLevel::Low:
            SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
            break;
        case PriorityLevel::Idle:
            SetThreadPriority(hThread, THREAD_PRIORITY_IDLE);
            break;
    }
#else
    switch (level) {
        case PriorityLevel::Normal:
            setpriority(PRIO_PROCESS, 0, 0);
            break;
        case PriorityLevel::Low:
            setpriority(PRIO_PROCESS, 0, 10);
            break;
        case PriorityLevel::Idle:
            setpriority(PRIO_PROCESS, 0, 19);
            break;
    }
#endif
}

void set_process_low_io_priority() {
#if defined(_WIN32) || defined(_WIN64)
    // Put current process into background mode (reduces CPU & I/O priority)
    SetPriorityClass(GetCurrentProcess(), PROCESS_MODE_BACKGROUND_BEGIN);
#else
    // Linux ioprio_set system call for IOPRIO_CLASS_IDLE
#if defined(SYS_ioprio_set)
    constexpr int IOPRIO_CLASS_IDLE = 3;
    constexpr int IOPRIO_WHO_PROCESS = 1;
    syscall(SYS_ioprio_set, IOPRIO_WHO_PROCESS, 0, (IOPRIO_CLASS_IDLE << 13));
#endif
#endif
}

double get_system_cpu_usage() {
    // Placeholder system CPU load metric
    return 0.0;
}

} // namespace os
} // namespace sfc
