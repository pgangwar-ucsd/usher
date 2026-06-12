#pragma once

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace ripples {
using variables_map = boost::program_options::variables_map;
using parsed_options = boost::program_options::parsed_options;

struct CLArgs {
    variables_map vm;
    parsed_options parsed;

    CLArgs(variables_map map, parsed_options options)
        : vm(map), parsed(options) {}
};

variables_map parse_argv(int argc, char *argv[]);

}; // namespace ripples
