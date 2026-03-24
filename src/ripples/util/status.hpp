#pragma once

namespace ripples::server {

enum class error_t : unsigned int {
    NO_ERROR = 0,
    EMPTY_TREE = 1,
    ROOT_NOT_FOUND = 2,
    NODE_NAME_NOT_FOUND = 3,
    NODE_ID_NOT_FOUND = 4,
};

struct Status {
    Status(error_t err) : error(err) {}
    operator bool() const { return error == error_t::NO_ERROR; }

    error_t error;
};
}; // namespace ripples::server

