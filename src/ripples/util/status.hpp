#pragma once

#include <array>
#include <boost/format.hpp>

namespace ripples::server {

enum class error_t : unsigned int {
    NO_ERROR = 0,
    EMPTY_TREE = 1,
    ROOT_NOT_FOUND = 2,
    NODE_NAME_NOT_FOUND = 3,
    NODE_ID_NOT_FOUND = 4,
    CANNOT_OPEN_LOG = 5
};

static const std::array<std::string, 6> ERROR_MSGS{
    "ripples: success",
    "ripples: input tree is empty",
    "ripples: input tree root not found",
    "ripples: node name %1% not found",
    "ripples: node id %1% not found",
    "ripples: unable to open final results output file"};

inline static std::string get_error(error_t err) {
    return std::string{
        ERROR_MSGS[static_cast<std::underlying_type_t<error_t>>(err)]};
}

// Helper to setup error message with proper node id information
inline static std::string init_error(error_t err, std::string_view node_id) {
    boost::format error_message = boost::format(get_error(err)) % node_id;
    return boost::str(error_message);
}

struct Status {
    Status(error_t err) : error(err), message(get_error(err)) {}
    Status(error_t err, std::string_view node_id)
        : error(err), message(init_error(err, node_id)) {}

    operator bool() const { return error == error_t::NO_ERROR; }
    const char *msg() { return message.c_str(); }

    error_t error;
    std::string message;
};
}; // namespace ripples::server

