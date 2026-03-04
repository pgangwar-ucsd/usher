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

struct MATCopyHelper {
    MATCopyHelper(const OptimizeMAT::Tree &tree) : tree(tree) {}
    // Helpers
    MAT::Mutation copy_mutation(const OptimizeMAT::Mutation &mutation) const;
    std::string get_node_name(OptimizeMAT::Node *node) const;
    MAT::Node *copy_root(OptimizeMAT::Node *root) const;

    std::optional<MAT::Tree> copy_to_mat(std::vector<MAT::Node> &preallocated_nodes) const;

    const OptimizeMAT::Tree &tree;
};

}; // namespace ripples::server

