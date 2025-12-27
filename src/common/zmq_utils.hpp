#pragma once

#include <zmq.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace surveillance {
namespace zmq_utils {

// Publish a JSON object to a ZMQ publisher socket
bool publish_json(zmq::socket_t& socket, const nlohmann::json& payload);

// Receive a JSON object from a ZMQ subscriber socket (non-blocking if specified)
std::optional<nlohmann::json> receive_json(zmq::socket_t& socket, bool wait = false);

// Binds or connects ensuring appropriate timeout settings
zmq::socket_t create_publisher(zmq::context_t& ctx, const std::string& endpoint, bool bind);
zmq::socket_t create_subscriber(zmq::context_t& ctx, const std::string& endpoint, bool bind);

} // namespace zmq_utils
} // namespace surveillance
