#include "ripples_server.hpp"
#include <iostream>

ripples::server::runner::runner(
    ripples::server::runner::InputTree &tree,
    const ripples::server::runner::MissingSamples &missing_samples)
    : proxy_tree_(tree), missing_samples_(missing_samples) {}

ripples::server::Status
ripples::server::runner::operator()(const ripples_parameters &params) {
    // Copy OptimizeMAT::MAT::Tree to MAT::Tree
    auto [opt_copied_tree, status] =
        proxy_tree_.clone(storage_, params.num_threads);
    if (!opt_copied_tree) {
        return status;
    }
    auto &copied_tree = opt_copied_tree.value();

    // Build Missing_Sample vector required by ripples from the names of
    // missing samples, and collect mutations from copied MAT tree
    // ie) basically copy from (vector<Sample_Muts>) to vector<Missing_Sample>
    auto [missing_names, error_node_id] =
        get_missing_sample_names(proxy_tree_.data(), missing_samples_);
    if (!missing_names) {
        return Status(error_t::NODE_ID_NOT_FOUND, error_node_id);
    }

    auto [missing_samples, error_node_name] =
        collect_missing_samples(copied_tree, missing_names.value());
    if (!missing_samples) {
        return Status(error_t::NODE_NAME_NOT_FOUND, error_node_name);
    }

    // Run ripples
    ripples::server::ripples_runner runner(copied_tree, missing_samples.value(),
                                           params);
    return runner();
}

