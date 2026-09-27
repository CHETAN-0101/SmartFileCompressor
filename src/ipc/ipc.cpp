#include "sfc/ipc.h"
#include <iostream>
#include <thread>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

namespace sfc {
namespace ipc {

IPCServer::IPCServer() = default;

IPCServer::~IPCServer() {
    stop();
}

bool IPCServer::start(MessageHandler handler) {
    handler_ = handler;
    running_ = true;

#if defined(_WIN32) || defined(_WIN64)
    std::thread([this]() {
        while (running_) {
            HANDLE hPipe = CreateNamedPipeA(
                WIN_PIPE_NAME,
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                4096,
                4096,
                0,
                NULL
            );

            if (hPipe == INVALID_HANDLE_VALUE) {
                break;
            }

            BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
            if (connected) {
                char buffer[2048] = {0};
                DWORD bytesRead = 0;
                if (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
                    std::string req(buffer, bytesRead);
                    std::string resp = handler_ ? handler_(req) : "OK";
                    DWORD bytesWritten = 0;
                    WriteFile(hPipe, resp.c_str(), static_cast<DWORD>(resp.size()), &bytesWritten, NULL);
                }
            }
            DisconnectNamedPipe(hPipe);
            CloseHandle(hPipe);
        }
    }).detach();
#else
    std::thread([this]() {
        int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_fd < 0) return;

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, NIX_SOCKET_PATH, sizeof(addr.sun_path) - 1);
        unlink(NIX_SOCKET_PATH);

        if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(server_fd);
            return;
        }

        listen(server_fd, 5);
        server_fd_ = server_fd;

        while (running_) {
            int client_fd = accept(server_fd, nullptr, nullptr);
            if (client_fd < 0) continue;

            char buffer[2048] = {0};
            ssize_t bytesRead = read(client_fd, buffer, sizeof(buffer) - 1);
            if (bytesRead > 0) {
                std::string req(buffer, bytesRead);
                std::string resp = handler_ ? handler_(req) : "OK";
                write(client_fd, resp.c_str(), resp.size());
            }
            close(client_fd);
        }
        close(server_fd);
        unlink(NIX_SOCKET_PATH);
    }).detach();
#endif

    return true;
}

void IPCServer::stop() {
    running_ = false;
}

std::string IPCClient::send_command(const std::string& request) {
#if defined(_WIN32) || defined(_WIN64)
    HANDLE hPipe = CreateFileA(
        WIN_PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        return "ERROR: SFC Daemon is not running.";
    }

    DWORD bytesWritten = 0;
    WriteFile(hPipe, request.c_str(), static_cast<DWORD>(request.size()), &bytesWritten, NULL);

    char buffer[2048] = {0};
    DWORD bytesRead = 0;
    ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
    CloseHandle(hPipe);

    return std::string(buffer, bytesRead);
#else
    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd < 0) return "ERROR: Cannot create socket.";

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, NIX_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(client_fd);
        return "ERROR: SFC Daemon is not running.";
    }

    write(client_fd, request.c_str(), request.size());
    char buffer[2048] = {0};
    ssize_t bytesRead = read(client_fd, buffer, sizeof(buffer) - 1);
    close(client_fd);
    return std::string(buffer, bytesRead);
#endif
}

} // namespace ipc
} // namespace sfc
