#pragma once

#include "arg_parse.hpp"
#include "ripples_search.hpp"

namespace ripples {
using string = std::string;

struct sample_params {
    const bool by_month;
    const string metadata_filepath;
    const string month;
    const string start_month;
    const string end_month;
};

struct simulation_params {
    uint32_t num_recombinants;
    uint32_t num_replicates;
    const string outdir;
    const string outfile;
    const string mutations_logfile;
    const string mat_outfile;
};

sample_params get_sample_parameters(const ripples::variables_map &vm);
simulation_params get_simulation_parameters(const ripples::variables_map &vm);
ripples::server::ripples_parameters
get_ripples_parameters(const ripples::variables_map &vm);

}; // namespace ripples
