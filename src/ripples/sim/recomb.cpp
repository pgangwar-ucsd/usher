#include "recomb.hpp"
#include "src/usher_common.hpp"
#include <stdexcept>

ripples::recomb::recomb(size_t count, int breakpoint_idx,
                        DonorAcceptorPair parents, MAT::Tree &tree)
    : breakpoint_idx_(breakpoint_idx), parents_(parents),
      id_("recomb_" + std::to_string(count)), tree_(tree),
      mutations_(apply_breakpoint(breakpoint_idx, parents)) {}

ripples::recomb::operator Missing_Sample() const {
    Missing_Sample sample(id_);
    sample.mutations = mutations_;
    return sample;
}

void print_mutations(const ripples::recomb::MutationsVec &mutations) {
    for (const auto &mut : mutations) {
        std::cout << mut.get_string() << '\n';
    }
}

void ripples::recomb::apply_mutations(
    const MAT::Tree &tree, const MAT::Node *node,
    ripples::recomb::MutationsVec &mutations,
    std::unordered_set<int> &positions,
    ripples::recomb::MutationPredicate &&pred) {

    auto node_to_root_path = tree.rsearch(node->identifier, true);
    for (const MAT::Node *curr : node_to_root_path) {
        for (auto mut : curr->mutations) {
            if (pred(mut.position)) {
                if ((mut.ref_nuc != mut.mut_nuc) &&
                    (positions.find(mut.position) == positions.end())) {
                    auto iter = std::lower_bound(mutations.begin(),
                                                 mutations.end(), mut);
                    auto m = mut.copy();
                    m.par_nuc = m.ref_nuc;
                    mutations.insert(iter, m);
                }
                positions.insert(mut.position);
            }
        }
    }
}

ripples::recomb::MutationsVec
ripples::recomb::apply_breakpoint(int breakpoint_idx,
                                  DonorAcceptorPair parents) {
    const auto [donor, acceptor] = parents;
    MutationsVec mutations;
    std::unordered_set<int> positions;
    std::cout << "Breakpoint position: " << breakpoint_idx << '\n';
    // Collect donor mutations up until breakpoint
    std::sort(donor->mutations.begin(), donor->mutations.end());
    // std::cout << "------------PRINTING DONOR MUTATIONS------------\n";
    print_mutations(donor->mutations);
    // std::cout << "------------------------------------------------\n";

    apply_mutations(tree_, donor, mutations, positions,
                    [breakpoint_idx](int pos) {
                        if (pos > breakpoint_idx) {
                            return false;
                        }
                        return true;
                    });

    // Then collect acceptor mutations
    std::sort(acceptor->mutations.begin(), acceptor->mutations.end());
    // std::cout << "------------PRINTING ACCEPTOR MUTATIONS------------\n";
    print_mutations(acceptor->mutations);
    // std::cout << "------------------------------------------------\n";

    apply_mutations(tree_, acceptor, mutations, positions,
                    [breakpoint_idx](int pos) {
                        if (pos <= breakpoint_idx) {
                            return false;
                        }
                        return true;
                    });
    // std::cout << "------------PRINTING RECOMB MUTATIONS------------\n";
    print_mutations(mutations);
    // std::cout << "------------------------------------------------\n";
    return mutations;
}

ripples::MissingSamples
ripples::recomb::make_replicates(size_t num_replicates) {
    MissingSamples samples;
    samples.reserve(1 + num_replicates);
    // Add recombinant, and num_replicates identical recombinants
    for (size_t i = 0; i < num_replicates + 1; ++i) {
        samples.emplace_back(static_cast<Missing_Sample>(*this));
    }
    return samples;
}

ripples::OptionalMissingSamples ripples::recomb::place(size_t num_replicates) {
    // NOTE: Unused, required for usher_common
    std::vector<std::string> low_confidence_samples;
    auto missing_samples = make_replicates(num_replicates);

    int return_val =
        usher_common("", ".", 1, 1e6, 1e6, false, false, false, false, false,
                     false, false, false, false, false, false, 0, 0,
                     missing_samples, low_confidence_samples, &tree_);
    if (return_val != 0) {
        std::cout << "Placement was not successful!\n";
        return std::nullopt;
    }
    return std::make_optional<MissingSamples>(missing_samples);
}

ripples::MissingSamples
ripples::recomb_collection::make_replicates(size_t num_replicates) {
    MissingSamples samples;
    // There will be num_replicates * num_recombinants + 1 missing samples
    samples.reserve(num_replicates * recombs_.size() + 1);
    for (const auto &recomb : recombs_) {
        for (size_t i = 0; i < num_replicates + 1; ++i) {
            // Explicit conversion from recombinant to MissingSample
            samples.emplace_back(recomb);
        }
    }
    return samples;
}

ripples::MissingSamples
ripples::recomb_collection::place_all(size_t num_replicates) {
    // NOTE: Unused, required for usher_common
    std::vector<std::string> low_confidence_samples;
    auto missing_samples = make_replicates(num_replicates);

    int return_val =
        usher_common("", outdir_, 1, 1e6, 1e6, false, false, false, false,
                     false, false, false, false, false, false, false, 0, 0,
                     missing_samples, low_confidence_samples, &tree_);
    if (return_val != 0) {
        throw std::runtime_error("Placing missing samples was unsuccessful.");
    }
    std::cerr << "Number of low confidence samples: "
              << low_confidence_samples.size() << '\n';
    return missing_samples;
}
