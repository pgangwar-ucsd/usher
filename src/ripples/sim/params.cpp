#include "params.hpp"

ripples::simulation_params
ripples::get_simulation_parameters(const ripples::variables_map &vm) {
    return {.num_recombinants = vm["num-recombinants"].as<uint32_t>(),
            .num_replicates = vm["num-replicates"].as<uint32_t>(),
            .outdir = vm["outdir"].as<std::string>(),
            .outfile = vm["outfile"].as<std::string>(),
            .mutations_logfile = vm["log-mutations"].as<std::string>(),
            .mat_outfile = vm["output-mat"].as<std::string>()};
}

ripples::sample_params
ripples::get_sample_parameters(const ripples::variables_map &vm) {
    return {.metadata_filepath = vm["input-metadata"].as<std::string>(),
            .month = vm["month"].as<std::string>(),
            .start_month = vm["start-month"].as<std::string>(),
            .end_month = vm["end-month"].as<std::string>()};
}

ripples::server::ripples_parameters
ripples::get_ripples_parameters(const ripples::variables_map &vm) {
    return {.branch_len = vm["branch-length"].as<uint32_t>(),
            .num_desc_search = vm["num-descendants-search"].as<uint32_t>(),
            .num_desc_recomb = vm["num-descendants-recomb"].as<uint32_t>(),
            .ancestor_radius = vm["ancestor-radius"].as<uint32_t>(),
            .num_threads = vm["threads"].as<uint32_t>(),
            .outdir = vm["outdir"].as<std::string>()};
}
