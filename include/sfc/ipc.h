#pragma once

#include <string>
#include <functional>

namespace sfc {
namespace ipc {

constexpr char WIN_PIPE_NAME[] = "\\\\.\\pipe\\sfc_daemon_pipe";
constexpr char NIX_SOCKET_PATH[] = "/tmp/sfc_daemon.sock";

class IPCServer {
public:
    using MessageHandler = std::function<std::string(const std::string&)>;

    IPCServer();
    ~IPCServer();

    bool start(MessageHandler handler);
    void stop();
    bool is_running() const { return running_; }

private:
    void listen_loop();

    MessageHandler handler_;
    bool running_ = false;
#if defined(_WIN32) || defined(_WIN64)
    void* server_pipe_ = nullptr;
#else
    int server_fd_ = -1;
#endif
};

class IPCClient {
public:
    static std::string send_command(const std::string& request);
};

} // namespace ipc
} // namespace sfc
