#pragma once

#include "src/ripples/util/logger.hpp"
#include "src/usher_graph.hpp"
#include <functional>
#include <optional>
#include <string_view>
#include <type_traits>

namespace ripples {

using DonorAcceptorPair = std::pair<MAT::Node *, MAT::Node *>;
using MissingSamples = std::vector<Missing_Sample>;
using OptionalMissingSamples = std::optional<MissingSamples>;

enum parent { DONOR = 0, ACCEPTOR = 1 };

class recomb {
  public:
    using string = std::string;
    using string_view = std::string_view;
    using MutationsVec = std::vector<MAT::Mutation>;
    using MutationPredicate = std::function<bool(int)>;

    recomb(size_t count, int breakpoint_idx, DonorAcceptorPair parents,
           MAT::Tree &tree);

    OptionalMissingSamples place(size_t num_replicates = 5);

    // recomb can convert itself to Missing_Sample
    explicit operator Missing_Sample() const;

    std::string_view name() const noexcept { return id_; }
    MutationsVec mutations() const noexcept { return mutations_; }
    size_t num_mutations() const noexcept { return mutations_.size(); }
    MAT::Node *get_donor() const noexcept { return std::get<0>(parents_); }
    MAT::Node *get_acceptor() const noexcept { return std::get<1>(parents_); }
    int breakpoint() const noexcept { return breakpoint_idx_; }

    // Each recomb can log itself to the mutations logfile
    friend void operator<<(std::ostream &os, const recomb &r) {
        auto donor = r.get_donor();
        auto acceptor = r.get_acceptor();
        //"DonorID\tDonorMutations\tAcceptorID\tAcceptorMutatio"
        //"ns\tBreakpoint\tRecombID\tRecomb"
        //"Mutations\n";

        // DonorID
        os << donor->identifier << '\t';
        size_t i{0};
        size_t donor_num_mutations = donor->mutations.size();
        // Num Donor Mutations
        os << donor_num_mutations << '\t';
        // Donor Mutations
        if (donor_num_mutations == 0) {
            os << "None\t";
        } else {
            for (const auto &mut : donor->mutations) {
                if (i + 1 != donor->mutations.size()) {
                    os << mut.get_string() << ",";
                } else {
                    os << mut.get_string() << "\t";
                }
                ++i;
            }
        }
        // AcceptorID
        os << acceptor->identifier << '\t';
        size_t acceptor_num_mutations = acceptor->mutations.size();
        // Num Acceptor Mutations
        os << acceptor_num_mutations << '\t';
        // Acceptor Mutations
        if (acceptor_num_mutations == 0) {
            os << "None\t";
        } else {
            for (const auto &mut : acceptor->mutations) {
                if (i + 1 != acceptor->mutations.size()) {
                    os << mut.get_string() << ",";
                } else {
                    os << mut.get_string() << "\t";
                }
                ++i;
            }
        }
        // Recomb ID
        os << r.name() << '\t';
        size_t recomb_num_mutations = acceptor->mutations.size();
        // Num Recomb Mutations
        os << recomb_num_mutations << '\t';
        // Recomb Mutations
        if (recomb_num_mutations == 0) {
            os << "None";
        } else {
            for (const auto &mut : r.mutations()) {
                if (i + 1 != r.num_mutations()) {
                    os << mut.get_string() << ",";
                } else {
                    os << mut.get_string() << "\t";
                }
                ++i;
            }
        }
        // Breakpoint
        os << '\t' << r.breakpoint() << '\n';
        // os << '\n';
    }

  private:
    MutationsVec apply_breakpoint(int breakpoint_idx,
                                  DonorAcceptorPair parents);

    MissingSamples make_replicates(size_t num_replicates);

    void apply_mutations(const MAT::Tree &tree, const MAT::Node *node,
                         ripples::recomb::MutationsVec &mutations,
                         std::unordered_set<int> &positions,
                         ripples::recomb::MutationPredicate &&pred);

    int breakpoint_idx_;
    DonorAcceptorPair parents_;
    string id_;
    MAT::Tree &tree_;
    std::vector<MAT::Mutation> mutations_;
};

class recomb_collection {
  public:
    explicit recomb_collection(MAT::Tree &tree, std::vector<recomb> recombs,
                               const std::string &outdir)
        : tree_(tree), recombs_(recombs), outdir_(outdir) {}

    explicit recomb_collection(MAT::Tree &&tree, std::vector<recomb> recombs,
                               const std::string &outdir)
        : tree_(tree), recombs_(recombs), outdir_(outdir) {}

    MissingSamples place_all(size_t num_replicates);

  private:
    MissingSamples make_replicates(size_t num_replicates);

    MAT::Tree &tree_;
    std::vector<recomb> recombs_;
    const std::string &outdir_;
};
}; // namespace ripples

