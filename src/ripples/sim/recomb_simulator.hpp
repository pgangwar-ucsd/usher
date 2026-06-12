#pragma once

#include "recomb.hpp"
#include "ripples_search.hpp"
#include "sample_collection.hpp"
#include <fstream>
#include <optional>
#include <unordered_set>

namespace ripples {

using NodePtrVec = std::vector<MAT::Node *>;

class simulator {
  public:
    friend class recomb;
    using string = std::string;
    using OptionalBranchLengths = std::optional<std::vector<float>>;

    simulator(MAT::Tree &tree, const ripples::simulation_params &params);
    ~simulator() = default;

    // Result type from running the simulation
    struct result {
        friend class simulator;
        using UniqueNodeIdCollection = std::unordered_set<std::string>;
        UniqueNodeIdCollection detected_recombs;
        std::string month;
        std::ofstream &os;

        void log() const;

        result(const std::string &month, std::ofstream &os)
            : month(month), os(os) {}
        ~result() = default;
        result(const result &) = delete;
        result &operator=(const result &) = delete;
        result(result &&) noexcept = delete;
        result &operator=(result &&) noexcept = delete;
    };

    recomb make_recombinant(const Samples &samples);
    recomb_collection make_recombinants(const Samples &samples,
                                        uint32_t num_recombinants);

    void search(const MissingSamples &samples,
                const server::ripples_parameters &params,
                const std::string &month);

    OptionalBranchLengths place(const recomb &recombinant,
                                size_t num_replicates = 5);

    void write_tree();

  private:
    DonorAcceptorPair select_random_pair(const Samples &samples);
    DonorAcceptorPair select_random_pair(const NodePtrVec &nodes);
    static size_t select_random_breakpoint();

    MAT::Tree get_tree() noexcept { return tree_; }

    static constexpr unsigned int GENOME_SIZE{29903};
    static constexpr std::string_view RECOMBINATION_RESULTS_BASE{
        "recombination_"};
    static constexpr std::string_view EXT{".tsv"};

    std::vector<std::string> create_months();
    std::vector<std::string> MONTHS = create_months();

    OptionalBranchLengths get_branch_lengths(const MissingSamples &recombs);

    MAT::Tree &tree_;
    const simulation_params &params_;
    size_t recomb_counter_;
};
}; // namespace ripples

