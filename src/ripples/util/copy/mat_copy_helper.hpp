#pragma once

#include "src/matOptimize/mutation_annotated_tree.hpp"

#include "src/usher-sampled/usher.hpp"
#include "src/usher_graph.hpp"
#include <iostream>
#include <optional>
#include <queue>
#include <src/mutation_annotated_tree.hpp>

namespace ripples::server {

// usher defined MAT Tree inferface
namespace MAT = Mutation_Annotated_Tree;
// matOptimize defined MAT Tree inferface
namespace OptimizeMAT = MatOptimize::Mutation_Annotated_Tree;

std::string get_node_name(const OptimizeMAT::Tree &tree,
                          const OptimizeMAT::Node *node);

struct MATCopyHelper {
    MATCopyHelper(const OptimizeMAT::Tree &tree)
        : tree(tree), num_nodes(tree.get_size_upper()) {}
    // Helpers
    inline MAT::Mutation
    copy_mutation(const OptimizeMAT::Mutation &mutation) const;
    std::optional<MAT::Node *> copy_root(OptimizeMAT::Node *root,
                                         std::vector<MAT::Node> &storage) const;

    std::optional<MAT::Tree>
    copy_to_mat(std::vector<MAT::Node> &preallocated_nodes) const;
    std::optional<MAT::Tree>
    copy_to_mat_parallel(std::vector<MAT::Node> &storage, uint32_t num_threads) const;

    const OptimizeMAT::Tree &tree;
    size_t num_nodes;
};

}; // namespace ripples::server

