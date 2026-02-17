#include "mat_proxy.hpp"

std::optional<std::vector<std::string>>
ripples::server::get_missing_sample_names(
    const MatOptimize::MAT::Tree &tree,
    const std::vector<Sample_Muts> &missing_samples) {
    std::vector<std::string> missing_sample_names;
    missing_sample_names.reserve(missing_samples.size());
    for (const auto &sample : missing_samples) {
        auto name = missing_sample_names.emplace_back(
            tree.get_node_name(sample.sample_idx));
        // TODO: How do we want to handle errors
        if (name.empty()) {
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
        sample.mutations = node->mutations;
    }
    // IMPORTANT NOTE:
    // Ignoring the following fields in Missing_Sample for now:
    // - best_clade_assignment
    // - clade_assignments
    // - num_ambiguous is set to 0 by Missing_Sample constructor
    return missing_samples;
}

ripples::server::MAT::Mutation ripples::server::MATCopyHelper::copy_mutation(
    const ripples::server::OptimizeMAT::Mutation &mutation) const {
    // NOTE: Chromosome field left as empty string.
    MAT::Mutation m;
    m.position = mutation.get_position();
    // MatOptimize mutation has conversion operators to unpack uint8_t
    m.ref_nuc = mutation.get_ref_one_hot();
    m.par_nuc = mutation.get_par_one_hot();
    m.mut_nuc = mutation.get_mut_one_hot();
    return m;
}

/*
ripples::server::MAT::Node *ripples::server::MATCopyHelper::copy_node(
    const std::string &identifier, MAT::Node *parent,
    OptimizeMAT::Node *node_to_copy) const {
    return nullptr;
}
*/



