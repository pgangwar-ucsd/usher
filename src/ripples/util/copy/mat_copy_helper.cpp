#include "mat_copy_helper.hpp"
std::optional<MAT::Tree> ripples::server::MATCopyHelper::copy_to_mat(std::vector<MAT::Node> &preallocated_nodes) const {
    Timer timer;
    timer.Start();
    std::optional<MAT::Tree> result(std::in_place);
    MAT::Tree &copied_tree = result.value();

    if (tree.get_size_upper() == 0) {
        std::cerr << "Tree size is empty!\n";
        return std::nullopt;
    }

    const size_t size_upper = tree.get_size_upper();
    copied_tree.all_nodes.reserve(size_upper);

    std::vector<std::pair<OptimizeMAT::Node*, int>> bfs_stack;
    std::vector<int> child_indices;
    std::vector<std::string> node_names(size_upper);
    std::vector<int> mut_counts(size_upper, 0);
    std::vector<int> child_counts(size_upper, 0);
    const auto& matoptimize_all_names = tree.get_node_names();

    bfs_stack.reserve(size_upper);
    child_indices.reserve(size_upper);
    bfs_stack.emplace_back(tree.root, -1);

    // Serial BFS: build traversal order to guide parallel copy, and collect mutation/child counts for pre-allocation
    for (size_t front = 0; front < bfs_stack.size(); ++front)
    {
        auto [matoptimize_curr, mat_parent] = bfs_stack[front];

        auto it = matoptimize_all_names.find(matoptimize_curr->node_id);
        node_names[front] = (it != matoptimize_all_names.end()) ? it->second : "node_" + std::to_string(matoptimize_curr->node_id);

        mut_counts[front] = (int)matoptimize_curr->mutations.mutations.size();

        if (matoptimize_curr->children.empty())
        {
            child_indices.emplace_back(-1);
        }
        else
        {
            child_counts[front] = (int)matoptimize_curr->children.size();
            child_indices.emplace_back((int)bfs_stack.size());
            for (const auto& c : matoptimize_curr->children)
            {
                bfs_stack.emplace_back(c, (int)front);
            }
        }
    }
	
    // Serial pre-allocation: resize mutations and children before parallel phase
    for (size_t k = 0; k < bfs_stack.size(); ++k)
    {
        if (mut_counts[k] > 0)
            preallocated_nodes[k].mutations.resize(mut_counts[k]);
        if (child_counts[k] > 0)
            preallocated_nodes[k].children.resize(child_counts[k]);
    }

    // Parallel fill
    static tbb::affinity_partitioner ap;
    tbb::parallel_for(tbb::blocked_range<size_t>(0, bfs_stack.size()),
    [&](tbb::blocked_range<size_t> r) {
        for (size_t k = r.begin(); k < r.end(); ++k) {
            auto [matoptimize_curr, mat_parent] = bfs_stack[k];
            auto c_idx_start = child_indices[k];
            MAT::Node *new_node = &preallocated_nodes[k];

            // Updating node identifier, branch length, and level
            new_node->identifier = std::move(node_names[k]);
            new_node->branch_length = static_cast<float>(matoptimize_curr->branch_length);
            new_node->level = matoptimize_curr->level;

            // Fill pre-sized children array
            if (c_idx_start > 0)
            {
                const int num_children = child_counts[k];
                for (int i = 0; i < num_children; i++)
                    new_node->children[i] = &preallocated_nodes[c_idx_start + i];
            }

            if (mat_parent == -1)
            {
                new_node->parent = nullptr;
                copied_tree.root = new_node;
            }
            else
            {
                new_node->parent = &preallocated_nodes[mat_parent];
            }

            // Fill pre-sized mutations array 
            const int num_mutations = mut_counts[k];
            for (int i = 0; i < num_mutations; i++)
            {
                auto& m = new_node->mutations[i];
                const auto& src_mut = matoptimize_curr->mutations.mutations[i];
                m.position = src_mut.get_position();
                m.ref_nuc = src_mut.get_ref_one_hot();
                m.par_nuc = src_mut.get_par_one_hot();
                m.mut_nuc = src_mut.get_mut_one_hot();
            }
        }
    }, ap);

    // Serial post-pass: populate all_nodes map after identifiers are finalized
    for (size_t k = 0; k < bfs_stack.size(); ++k)
    {
        MAT::Node* node = &preallocated_nodes[k];
        copied_tree.all_nodes[node->identifier] = node;
    }

	// TESTING
    std::cerr << "Finished tree copy in mat_proxy clone\n";
    std::cerr << "Original Root Children: " << tree.root->children.size()
              << "\n";
    std::cerr << "New Root Children: " << copied_tree.root->children.size()
              << "\n";
	std::cerr << "Tree copied in: " << timer.Stop() << " msec \n";
    return result;
}
