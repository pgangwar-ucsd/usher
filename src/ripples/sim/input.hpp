#pragma once

#include "arg_parse.hpp"
#include <string>
#include <string_view>

namespace ripples {

struct input_context {
    input_context(std::string_view mat_file, std::string_view metadata_file)
        : mat_file(mat_file), metadata_file(metadata_file) {}
    input_context(variables_map &vm)
        : mat_file(vm["input-mat"].as<std::string>()),
          metadata_file(vm["input-metadata"].as<std::string>()) {}

    const std::string mat_file;
    const std::string metadata_file;
};


}; // namespace ripples
