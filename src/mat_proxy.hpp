#pragma once

#include "src/matOptimize/mutation_annotated_tree.hpp"
#include "src/usher-sampled/usher.hpp"
#include "src/usher_graph.hpp"
#include <iostream>
#include <optional>
#include <src/mutation_annotated_tree.hpp>
#include <stack>

namespace ripples::server {

// usher defined MAT Tree inferface
namespace MAT = Mutation_Annotated_Tree;
// matOptimize defined MAT Tree inferface
namespace OptimizeMAT = MatOptimize::Mutation_Annotated_Tree;

template <class T> struct is_tree_t {
    static_assert(std::is_same_v<std::decay_t<T>, MAT::Tree> ||
                      std::is_same_v<std::decay_t<T>, OptimizeMAT::Tree>,
                  "Type given to MATProxy is not a valid tree type.");
};

struct MATCopyHelper {
    // MAT::Node *copy_node(const std::string &identifier, MAT::Node *parent,
    // OptimizeMAT::Node *node_to_copy) const;
    MAT::Mutation copy_mutation(const OptimizeMAT::Mutation &mutation) const;
};

// Usage:
// MAT::Tree tree;
// MATProxy proxy_tree(tree);
// auto opt_copied_tree = proxy_tree.clone();
// if (!opt_copied_tree) {
// 	...handle error that occurred during tree copy
// }
// auto copied_tree = opt_copied_tree.value();

template <class T> class MATProxy final : is_tree_t<T> {
  public:
    MATProxy(const T &tree) : tree_(tree) {}

    // Getter for underlying tree held
    const auto &data() const noexcept { return tree_; }

    auto clone() const {
        using tree_t = std::decay_t<T>;
        // If given tree type is MatOptimize::MAT, convert to MAT
        if constexpr (std::is_same_v<tree_t, MatOptimize::MAT::Tree>) {
            return copy_matoptimize_to_mat();
        }
        // Otherwise underlying tree type if MAT, conver to MatOptimize::MAT
        else {
            return copy_mat_to_matoptimize();
        }
    }

  private:
    const T &tree_;

    std::optional<MAT::Tree> copy_matoptimize_to_mat() const {
        MAT::Tree copied_tree;
        if (tree_.get_size_upper() == 0) {
            return std::nullopt;
        }
        MATCopyHelper helper;

        auto *root = tree_.root;
        if (!root) {
            return std::nullopt;
        }

        // Create a copied root node, and assign new root in copied tree
        // TODO: Check these construction parameters for root are correct
        auto *new_root = copied_tree.create_node(
            tree_.get_node_name(root->node_id), nullptr, -1);
        copied_tree.root = new_root;

        struct NodeParentPair {
            OptimizeMAT::Node *curr_node;
            MAT::Node *parent_node;
        };

        std::stack<NodeParentPair> bfs_order;
        // Get all OptimizeMAT children of root
        for (auto *child : root->children) {
            bfs_order.push(NodeParentPair{child, new_root});
        }

        while (!bfs_order.empty()) {
            // Current node is OptimizeMAT node, while parent_node is copied
            // node already in copied_tree
            auto [curr_node, parent_node] = bfs_order.top();
            bfs_order.pop();

            // Create a new node in the copied tree, copying from OptimizeMAT
            // current node
            auto identifier = tree_.get_node_name(curr_node->node_id);
            // Branch length is type int in OptimizeMAT but float in MAT
            auto bl = static_cast<float>(curr_node->branch_length);
            // NOTE: create_node updates all_nodes internally
            auto *new_node =
                copied_tree.create_node(identifier, parent_node, bl);
            // Copy clade annotations
            new_node->clade_annotations = curr_node->clade_annotations;
            // Copy additional bfs index metadata
            new_node->level = curr_node->level;
            new_node->dfs_idx = curr_node->dfs_index;
            new_node->dfs_end_idx = curr_node->dfs_end_index;

            // Copy mutations
            size_t num_mutations = curr_node->mutations.mutations.size();
            new_node->mutations.reserve(num_mutations);
            for (const auto &mut : curr_node->mutations.mutations) {
                new_node->mutations.emplace_back(helper.copy_mutation(mut));
            }
            // Add new node as child of previous parent node
            parent_node->children.push_back(new_node);

            // Collect all the children of current OptimizeMAT nodes
            for (auto *child : curr_node->children) {
                // Child is OptimizeMAT node, but parent is new_node
                // in copied tree,
                // otherwise link to parent in copied tree will be broken
                bfs_order.push(NodeParentPair{child, new_node});
            }
        }
        return std::make_optional<MAT::Tree>(copied_tree);
    }

    std::optional<MatOptimize::MAT::Tree> copy_mat_to_matoptimize() const {
        // TODO: Currently unused by any workflow
        // Needs further testing before use
        // TODO: implement copy_mat_to_matoptimize
        return std::nullopt;
    }
};
}; // namespace ripples::server

