#include "mat_copy_helper.hpp"

std::optional<MAT::Tree> ripples::server::MATCopyHelper::copy_to_mat(
    std::vector<MAT::Node> &preallocated_nodes) const {
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

    std::vector<std::pair<OptimizeMAT::Node *, int>> bfs_stack;
    std::vector<int> child_indices;
    std::vector<std::string> node_names(size_upper);
    std::vector<int> mut_counts(size_upper, 0);
    std::vector<int> child_counts(size_upper, 0);
    const auto &matoptimize_all_names = tree.get_node_names();

    bfs_stack.reserve(size_upper);
    child_indices.reserve(size_upper);
    bfs_stack.emplace_back(tree.root, -1);

    // Serial BFS: build traversal order to guide parallel copy, and collect
    // mutation/child counts for pre-allocation
    for (size_t front = 0; front < bfs_stack.size(); ++front) {
        auto [matoptimize_curr, mat_parent] = bfs_stack[front];

        auto it = matoptimize_all_names.find(matoptimize_curr->node_id);
        node_names[front] =
            (it != matoptimize_all_names.end())
                ? it->second
                : "node_" + std::to_string(matoptimize_curr->node_id);

        mut_counts[front] = (int)matoptimize_curr->mutations.mutations.size();

        if (matoptimize_curr->children.empty()) {
            child_indices.emplace_back(-1);
        } else {
            child_counts[front] = (int)matoptimize_curr->children.size();
            child_indices.emplace_back((int)bfs_stack.size());
            for (const auto &c : matoptimize_curr->children) {
                bfs_stack.emplace_back(c, (int)front);
            }
        }
    }

    // Serial pre-allocation: resize mutations and children before parallel
    // phase
    for (size_t k = 0; k < bfs_stack.size(); ++k) {
        if (mut_counts[k] > 0)
            preallocated_nodes[k].mutations.resize(mut_counts[k]);
        if (child_counts[k] > 0)
            preallocated_nodes[k].children.resize(child_counts[k]);
    }

    // Parallel fill
    static tbb::affinity_partitioner ap;
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, bfs_stack.size()),
        [&](tbb::blocked_range<size_t> r) {
            for (size_t k = r.begin(); k < r.end(); ++k) {
                auto [matoptimize_curr, mat_parent] = bfs_stack[k];
                auto c_idx_start = child_indices[k];
                MAT::Node *new_node = &preallocated_nodes[k];

                // Updating node identifier, branch length, and level
                new_node->identifier = std::move(node_names[k]);
                new_node->branch_length =
                    static_cast<float>(matoptimize_curr->branch_length);
                new_node->level = matoptimize_curr->level;

                // Fill pre-sized children array
                if (c_idx_start > 0) {
                    const int num_children = child_counts[k];
                    for (int i = 0; i < num_children; i++)
                        new_node->children[i] =
                            &preallocated_nodes[c_idx_start + i];
                }

                if (mat_parent == -1) {
                    new_node->parent = nullptr;
                    copied_tree.root = new_node;
                } else {
                    new_node->parent = &preallocated_nodes[mat_parent];
                }

                // Fill pre-sized mutations array
                const int num_mutations = mut_counts[k];
                for (int i = 0; i < num_mutations; i++) {
                    auto &m = new_node->mutations[i];
                    const auto &src_mut =
                        matoptimize_curr->mutations.mutations[i];
                    m.position = src_mut.get_position();
                    m.ref_nuc = src_mut.get_ref_one_hot();
                    m.par_nuc = src_mut.get_par_one_hot();
                    m.mut_nuc = src_mut.get_mut_one_hot();
                }
            }
        },
        ap);

    // Serial post-pass: populate all_nodes map after identifiers are finalized
    for (size_t k = 0; k < bfs_stack.size(); ++k) {
        MAT::Node *node = &preallocated_nodes[k];
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

std::string
ripples::server::get_node_name(const ripples::server::OptimizeMAT::Tree &tree,
                               const ripples::server::OptimizeMAT::Node *node) {
    if (!node) {
        return "";
    }
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

std::optional<ripples::server::MAT::Node *>
ripples::server::MATCopyHelper::copy_root(
    OptimizeMAT::Node *root,
    std::vector<ripples::server::MAT::Node> &storage) const {
    auto identifier = get_node_name(tree, tree.root);
    if (identifier.empty()) {
        return std::nullopt;
    }
    return &storage.emplace_back(identifier, nullptr,
                                 static_cast<float>(tree.root->branch_length),
                                 tree.root->level);
}

std::optional<MAT::Tree> ripples::server::MATCopyHelper::copy_to_mat_parallel(
    std::vector<MAT::Node> &storage, uint32_t num_threads) const {

    if (num_nodes == 0) {
        std::cerr << "MatOptimize Tree is empty!\n";
        return std::nullopt;
    }
    storage.reserve(num_nodes);

    tbb::global_control global_limit(
        tbb::global_control::max_allowed_parallelism, num_threads);
    srand(time(NULL));
    static tbb::affinity_partitioner ap;

    Timer timer;
    timer.Start();

    MAT::Tree copied_tree(num_nodes);
    auto &all_nodes = copied_tree.get_all_nodes();

    struct NodeParentPair {
        OptimizeMAT::Node *curr_node;
        MAT::Node *parent_node;
    };

    using NodeParentPairVec = std::vector<NodeParentPair>;
    NodeParentPairVec current_level;
    NodeParentPairVec next_level;
    next_level.reserve(num_nodes);

    // Copy and update root node in new tree
    auto new_root_opt = copy_root(tree.root, storage);
    if (!new_root_opt) {
        return std::nullopt;
    }
    auto *new_root = new_root_opt.value();
    copied_tree.root = new_root;
    all_nodes.try_emplace(new_root->identifier, new_root);

    for (auto *child : tree.root->children) {
        current_level.push_back({child, new_root});
    }
    // level_offset keeps track of the level starting offset into all_nodes
    // Initialize as 1, since root node was already inserted
    size_t level_offset = 1;

    using all_nodes_t = std::unordered_map<std::string, MAT::Node *>;
    tbb::enumerable_thread_specific<all_nodes_t> local_all_nodes_maps;

    // Start from level 1 (root children) and work down to tip samples level
    while (!current_level.empty()) {
        size_t current_level_size = current_level.size();
        next_level.clear();

        // Copy all the nodes on a particular level of the MATOptimize tree
        // This will copy current_level_size nodes into storage
        tbb::parallel_for(
            tbb::blocked_range<size_t>(0, current_level_size),
            [&](tbb::blocked_range<size_t> r) {
                all_nodes_t &local_all_nodes = local_all_nodes_maps.local();

                for (size_t i = r.begin(); i < r.end(); ++i) {
                    auto [matoptimize_curr, mat_parent] = current_level[i];

                    auto identifier = get_node_name(tree, matoptimize_curr);
                    MAT::Node *new_node = storage.data() + level_offset + i;
                    ::new (new_node) MAT::Node(
                        identifier, mat_parent,
                        static_cast<float>(matoptimize_curr->branch_length),
                        matoptimize_curr->level);

                    local_all_nodes.try_emplace(identifier, new_node);

                    // Copy mutations
                    size_t num_mutations =
                        matoptimize_curr->mutations.mutations.size();
                    new_node->mutations.reserve(num_mutations);
                    for (const auto &mut :
                         matoptimize_curr->mutations.mutations) {
                        new_node->mutations.emplace_back(copy_mutation(mut));
                    }
                }
            });

        // Serially add children nodes to parent, and setup next level
        for (size_t i = 0; i < current_level_size; ++i) {
            auto [matoptimize_node, mat_parent] = current_level[i];
            auto *copied_node = &storage[level_offset + i];
            // Link to parent
            mat_parent->children.push_back(copied_node);
            for (auto *child : matoptimize_node->children) {
                next_level.push_back({child, copied_node});
            }
        }

        // Merge thread local all nodes maps merging into global all nodes map
        for (auto &local_map : local_all_nodes_maps) {
            all_nodes.merge(local_map);
            local_map.clear();
        }
        // Setup next level of nodes to copy from matoptimize tree
        level_offset += current_level_size;
        std::swap(current_level, next_level);
    }

    // TESTING
    std::cerr << "Tree copied in: " << timer.Stop() << " msec \n";
    std::cerr << "Finished tree copy in mat_proxy clone\n";
    std::cerr << "Original Root Children: " << tree.root->children.size()
              << "\n";
    std::cerr << "New Root Children: " << copied_tree.root->children.size()
              << "\n";
    return std::make_optional<MAT::Tree>(copied_tree);
}

