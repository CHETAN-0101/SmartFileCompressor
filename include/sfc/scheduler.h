#pragma once

#include "sfc/thread_pool.h"

namespace sfc {
namespace os {

// Sets current thread CPU priority according to OS API
void set_current_thread_priority(PriorityLevel level);

// Sets process-wide background I/O priority
void set_process_low_io_priority();

// Returns current system CPU usage percentage (0-100)
double get_system_cpu_usage();

} // namespace os
} // namespace sfc
