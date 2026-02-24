#include "mat_copy_helper.hpp"

ripples::server::MAT::Mutation ripples::server::MATCopyHelper::copy_mutation(
    const OptimizeMAT::Mutation &mutation) const {
    // NOTE: Chromosome field left as empty string.
    MAT::Mutation m;
    m.position = mutation.get_position();
    // MatOptimize mutation has conversion operators to unpack uint8_t
    m.ref_nuc = mutation.get_ref_one_hot();
    m.par_nuc = mutation.get_par_one_hot();
    m.mut_nuc = mutation.get_mut_one_hot();
    return m;
}

ripples::server::MAT::Node *
ripples::server::MATCopyHelper::copy_root(OptimizeMAT::Node *root) const {
    MAT::Node *new_root = new MAT::Node();
    new_root->identifier = get_node_name(tree.root);
    new_root->branch_length = static_cast<float>(tree.root->branch_length);
    new_root->level = tree.root->level;
    new_root->parent = nullptr;
    return new_root;
}

std::string
ripples::server::MATCopyHelper::get_node_name(OptimizeMAT::Node *node) const {
    auto nid = node->node_id;
    auto identifier = tree.get_node_name(nid);
    // Check if node is a sample node
    // MATOptimize only stores sample names
    if (!identifier.empty()) {
        return identifier;
    }
    // Otherwise, get internal node id name
    return "node_" + std::to_string(nid);
}

std::optional<MAT::Tree> ripples::server::MATCopyHelper::copy_to_mat() const {
		Timer timer;
		timer.Start();
    MAT::Tree copied_tree;
    if (tree.get_size_upper() == 0) {
        std::cerr << "Tree size is empty!\n";
        return std::nullopt;
    }
    struct NodeParentPair {
        OptimizeMAT::Node *curr_node;
        MAT::Node *parent_node;
    };
    std::queue<NodeParentPair> remaining_nodes;

    // Copy and update root node in new tree
    auto *new_root = copy_root(tree.root);
    copied_tree.root = new_root;
    copied_tree.update_all_nodes(new_root);
    for (auto *child : tree.root->children) {
        remaining_nodes.push({child, new_root});
    }

    while (!remaining_nodes.empty()) {
        auto [matoptimize_curr, mat_parent] = remaining_nodes.front();
        remaining_nodes.pop();

        MAT::Node *new_node = new MAT::Node();
        // Copy standard attributes
        new_node->identifier = get_node_name(matoptimize_curr);
        new_node->branch_length =
            static_cast<float>(matoptimize_curr->branch_length);
        new_node->level = matoptimize_curr->level;
        // Link to parent in newly created MAT tree
        new_node->parent = mat_parent;
        mat_parent->children.push_back(new_node);

        // Copy mutations
        size_t num_mutations = matoptimize_curr->mutations.mutations.size();
        new_node->mutations.reserve(num_mutations);
        for (const auto &mut : matoptimize_curr->mutations.mutations) {
            new_node->mutations.emplace_back(copy_mutation(mut));
        }
        // Register node
        copied_tree.update_all_nodes(new_node);
        for (auto *child : matoptimize_curr->children) {
            remaining_nodes.push({child, new_node});
        }
    }
		// TESTING
    std::cerr << "Finished tree copy in mat_proxy clone\n";
    std::cerr << "Original Root Children: " << tree.root->children.size()
              << "\n";
    std::cerr << "New Root Children: " << copied_tree.root->children.size()
              << "\n";
		std::cerr << "Tree copied in: " << timer.Stop() << " msec \n";
    return std::make_optional<MAT::Tree>(copied_tree);
}
