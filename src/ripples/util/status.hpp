#pragma once

#include <array>

namespace ripples::server {

enum class error_t : unsigned int {
    NO_ERROR = 0,
    EMPTY_TREE = 1,
    ROOT_NOT_FOUND = 2,
    NODE_NAME_NOT_FOUND = 3,
    NODE_ID_NOT_FOUND = 4,
};

static constexpr std::array<const char *, 5> ERROR_MSGS{
    "ripples: success", "ripples: input tree is empty",
    "ripples: input tree root not found", "ripples: node name not found",
    "ripples: node id not found"};

struct Status {
    Status(error_t err) : error(err) {}
    operator bool() const { return error == error_t::NO_ERROR; }

    const char *msg() const {
        return ERROR_MSGS[static_cast<std::underlying_type_t<error_t>>(error)];
    }

    error_t error;
};
}; // namespace ripples::server

