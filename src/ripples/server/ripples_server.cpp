#include "ripples_server.hpp"
#include <iostream>

ripples::server::runner::runner(
    ripples::server::runner::InputTree &tree,
    const ripples::server::runner::MissingSamples &missing_samples, std::vector<MAT::Node> &preallocated_nodes)
    : proxy_tree_(tree), missing_samples_(missing_samples), preallocated_nodes_(preallocated_nodes) {}

bool ripples::server::runner::operator()(const ripples_parameters &params) {
    // Copy OptimizeMAT::MAT::Tree to MAT::Tree
    auto opt_copied_tree = proxy_tree_.clone(preallocated_nodes_);
    if (!opt_copied_tree) {
        std::cerr
            << "[Error] Failed to copy MatOptimize::MAT::Tree to MAT::Tree\n";
        return false;
    }
    auto &copied_tree = opt_copied_tree.value();

    // Build Missing_Sample vector required by ripples from the names of
    // missing samples, and collect mutations from copied MAT tree
    // ie) basically copy from (vector<Sample_Muts>) to vector<Missing_Sample>
    auto missing_names =
        get_missing_sample_names(proxy_tree_.data(), missing_samples_);
    if (!missing_names) {
        std::cerr << "[Error] Node id not found in original "
                     "MatOptimize::MAT::Tree.\n";
        return false;
    }
    auto missing_samples =
        collect_missing_samples(copied_tree, missing_names.value());
    if (!missing_samples) {
        std::cerr << "[Error] Node name not found in copied MAT::Tree.\n";
        return false;
    }

    // Run ripples
    ripples::server::ripples_runner runner(copied_tree, missing_samples.value(),
                                           params);
    return runner();
}

