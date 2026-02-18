#include "ripples_server_util.hpp"
#include <iostream>

std::optional<std::vector<std::string>>
ripples::server::get_missing_sample_names(
    const MatOptimize::MAT::Tree &tree,
    const std::vector<Sample_Muts> &missing_samples) {
    std::vector<std::string> missing_sample_names;
    missing_sample_names.reserve(missing_samples.size());
    for (const auto &sample : missing_samples) {
        auto name = missing_sample_names.emplace_back(
            tree.get_node_name(sample.sample_idx));
        if (name.empty()) {
            std::cerr << "[Error] Node id " << sample.sample_idx
                      << " not found in tree\n";
            return std::nullopt;
        }
    }
    return missing_sample_names;
}

std::optional<std::vector<Missing_Sample>>
ripples::server::collect_missing_samples(
    const ripples::server::MAT::Tree &tree,
    const std::vector<std::string> &missing_sample_names) {

    std::vector<Missing_Sample> missing_samples;
    for (const auto &name : missing_sample_names) {
        auto &sample = missing_samples.emplace_back(Missing_Sample(name));
        const auto *node = tree.get_node(name);
        if (!node) {
            return std::nullopt;
        }
        sample.mutations = node->mutations;
    }
    // IMPORTANT NOTE:
    // Ignoring the following fields in Missing_Sample for now:
    // - best_clade_assignment
    // - clade_assignments
    // - num_ambiguous is set to 0 by Missing_Sample constructor
    return missing_samples;
}

