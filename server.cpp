#include <cstdint>
#include <iostream>
#include <memory>
#include <string_view>
#include <type_traits>

#include <linux/netlink.h>

#include <netlink/msg.h>
#include <netlink/socket.h>

#include "json.hpp"

namespace
{
    template <typename T>
    constexpr bool alwaysFalseValue = false;
}

int main()
{
    const std::unique_ptr<nl_sock, void (*)(nl_sock*)> sock{nl_socket_alloc(), nl_socket_free};
    if (!sock) {
        std::cerr << "nl_socket_alloc() failed\n";
        return -1;
    }

    if (const int res = nl_socket_modify_cb(sock.get(), NL_CB_VALID, NL_CB_CUSTOM, [](nl_msg *msg, void *sock) -> int {
        const auto sendResponse = [sock = static_cast<nl_sock*>(sock), peerPort = nlmsg_get_src(msg)->nl_pid]<typename T>(const T response) {
            std::string dump;

            if constexpr (std::is_same_v<T, double>) {
                dump = nlohmann::json::object({{"result", response}}).dump(1);
            } else if constexpr (std::is_same_v<T, const char *>) {
                dump = nlohmann::json::object({{"error", response}}).dump(1);
            } else {
                static_assert(alwaysFalseValue<T>, "Unsupported response type");
            }

            nl_socket_set_peer_port(sock, peerPort);

            if (const int res = nl_send_simple(sock, 0, 0, dump.data(), dump.length()); res < 0) {
                std::cerr << "nl_send_simple() failed: " << nl_geterror(res) << "\n";
            }
        };

        const nlmsghdr *hdr = nlmsg_hdr(msg);

        const int dataLen = nlmsg_datalen(hdr);
        if (dataLen < 0) {
            sendResponse("Invalid request");
            return NL_OK;
        }

        try {
            const auto jsonReq = nlohmann::json::parse(std::string_view{static_cast<const char*>(nlmsg_data(hdr)), static_cast<std::size_t>(dataLen)});

            const double firstArg = jsonReq.at("argument_1");
            const double secondArg = jsonReq.at("argument_2");
            const std::string &action = jsonReq.at("action");

            if (action == "add") {
                sendResponse(firstArg + secondArg);
            } else if (action == "sub") {
                sendResponse(firstArg - secondArg);
            } else if (action == "mul") {
                sendResponse(firstArg * secondArg);
            } else if (action == "div") {
                sendResponse(firstArg / secondArg);
            } else {
                sendResponse("Invalid action");
            }
        }
        catch (...) {
            sendResponse("Invalid json");
        }

        return NL_OK;
    }, sock.get()); res < 0) {
        std::cerr << "nl_socket_modify_cb() failed: " << nl_geterror(res) << "\n";
        return -1;
    }

    nl_socket_disable_seq_check(sock.get());

    if (const int res = nl_connect(sock.get(), NETLINK_GENERIC); res < 0) {
        std::cerr << "nl_connect() failed: " << nl_geterror(res) << "\n";
        return -1;
    }

    std::cout << "Listening on port " << nl_socket_get_local_port(sock.get()) << " ...\n";

    while (true) {
        if (const int res = nl_recvmsgs_default(sock.get()); res < 0) {
            std::cerr << "nl_recvmsgs_default() failed: " << nl_geterror(res) << "\n";
            return -1;
        }
    }

    return 0;
}
