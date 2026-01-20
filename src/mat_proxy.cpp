#include "mat_proxy.h"

ripples::server::MAT::Node *
copy_node(ripples::server::MAT::Tree &tree, ripples::server::MAT::Node *parent,
          ripples::server::OptimizeMAT::Node *node) {
    // if (all_nodes.find(identifier) != all_nodes.end()) {
    // fprintf(stderr, "Error: %s already in the tree!\n",
    // identifier.c_str());
    // exit(1);
    //}
    const std::string identifier = tree.new_internal_node_id();
    auto bl = static_cast<float>(node->branch_length);

    auto *new_node = new MAT::Node(identifier, parent, bl);
    size_t num_annotations = node->clade_annotations.size();
    new_node->clade_annotations.reserve(num_annotations);
    new_node->clade_annotations(node->clade_annotations);
    new_node->level = node->level;
    new_node->dfs_idx = node->dfs_index;
    new_node->dfs_end_idx = node->dfs_end_index;

    size_t num_mutations = node->mutations.mutations.size();
    new_node->mutations.reserve(num_mutations);
    for (const auto &mut : node->mutations.mutations) {
        new_node->mutations.emplace_back(copy_mutation(mut));
    }
    return new_node;
}

// TODO: Needs testing
// NOTE: Chromosome field left as empty string.
ripples::server::MAT::Mutation
copy_mutation(ripples::server::OptimizeMAT::Mutation &mutation) {
    MAT::Mutation m;
    m.position = mutation.position;
    // MatOptimize mutation has conversion operators to unpack uint8_t
    m.ref_nuc = mutation.get_ref_one_hot();
    m.par_nuc = mutation.get_par_one_hot();
    m.mut_nuc = mutation.get_mut_one_hot();
    return m;
}

static ripples::server::MATProxy::OptionalTree
ripples::server::copy_matoptimize_to_mat(const OptimizeMAT::Tree &tree) {
    MAT::Tree tree;
    if (tree_.all_nodes.size() == 0) {
        // TODO: warn empty tree, error?
        return std::nullopt;
        // return tree;
    }
    // all_nodes, including leaf nodes, stored in dfs order
    auto *root = tree_.all_nodes[0];
    if (!root || root != tree_.root) {
        return std::nullopt;
    }
    auto *new_root = copy_node(tree, nullptr, root);
    MAT::Node *parent = new_root;

    struct NodeParentPair {
        OptimizeMAT::Node *curr_node;
        MAT::Node *parent_node;
    };

    std::stack<NodeParentPair> dfs_order;
    while (!dfs_order.empty()) {
        auto [curr_node, parent_node] = dfs_order.top();
        dfs_order.pop();

        auto *new_node = copy_node(tree, parent_node, curr_node);
        // If new node has parent (not root), add node to parent's children
        if (parent_node) {
            parent->children.push_back(new_node);
        }
        // TODO: Make sure root is set.
        tree.all_nodes[new_node->identifier] = new_node;
        for (const auto &child : curr_node->children) {
            dfs_order.push({child, curr_node});
        }
    }
    return std::make_optional<MAT::Tree>(tree);
}

// TODO: implement copy_mat_to_matoptimize

