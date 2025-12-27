#include "zmq_utils.hpp"
#include <iostream>

namespace surveillance {
namespace zmq_utils {

bool publish_json(zmq::socket_t& socket, const nlohmann::json& payload) {
    std::string str = payload.dump() + "\n";
    zmq::message_t msg(str.data(), str.size());
    try {
        auto res = socket.send(msg, zmq::send_flags::none);
        return res.has_value();
    } catch (const zmq::error_t& e) {
        return false;
    }
}

std::optional<nlohmann::json> receive_json(zmq::socket_t& socket, bool wait) {
    zmq::message_t msg;
    try {
        auto flags = wait ? zmq::recv_flags::none : zmq::recv_flags::dontwait;
        auto res = socket.recv(msg, flags);
        if (res.has_value()) {
            std::string str(static_cast<char*>(msg.data()), msg.size());
            return nlohmann::json::parse(str);
        }
    } catch (...) {
        // Drop on parse error or network error
    }
    return std::nullopt;
}

zmq::socket_t create_publisher(zmq::context_t& ctx, const std::string& endpoint, bool bind) {
    zmq::socket_t socket(ctx, zmq::socket_type::pub);
    socket.set(zmq::sockopt::sndhwm, 10000);
    if (bind) {
        socket.bind(endpoint);
    } else {
        socket.connect(endpoint);
    }
    return socket;
}

zmq::socket_t create_subscriber(zmq::context_t& ctx, const std::string& endpoint, bool bind) {
    zmq::socket_t socket(ctx, zmq::socket_type::sub);
    socket.set(zmq::sockopt::rcvhwm, 10000);
    socket.set(zmq::sockopt::subscribe, "");
    if (bind) {
        socket.bind(endpoint);
    } else {
        socket.connect(endpoint);
    }
    return socket;
}

} // namespace zmq_utils
} // namespace surveillance
