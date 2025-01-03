#pragma once

#include <memory>
#include <functional>
#include <stddef.h>

namespace moony {
class buffer;
class tcp_connection;
class time_stamp;

using tcp_connection_ptr = std::shared_ptr<tcp_connection>;

using connection_callback = std::function<void(const tcp_connection_ptr&)>;

using close_callback = std::function<void(const tcp_connection_ptr&)>;

using write_complete_callback = std::function<void(const tcp_connection_ptr&)>;

using message_callback = std::function<void(const tcp_connection_ptr&, 
                                                buffer*, 
                                                time_stamp)>;
using high_water_mark_callback = std::function<void(const tcp_connection_ptr&, size_t)>;
}